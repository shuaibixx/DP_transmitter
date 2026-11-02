"""Check working files or Git blobs before publishing; standard library only."""
import argparse
import pathlib
import re
import subprocess
import sys
import zipfile
import io

ENCRYPTED_HEADER = bytes.fromhex('17da5fa01633cd9abc5b6cbf14aa1e33')
TEXT_EXTENSIONS = {
    '.c', '.h', '.s', '.asm', '.inc', '.md', '.txt', '.json', '.xml', '.csv',
    '.tsv', '.py', '.js', '.cjs', '.ps1', '.sh', '.bat', '.cmd', '.ini', '.cfg',
    '.uvprojx', '.uvoptx', '.uvproj', '.uvopt', '.uvmpw', '.sct', '.scvd',
    '.htm', '.html', '.css', '.map', '.log', '.hex', '.dep', '.icf', '.ld',
    '.ewp', '.eww', '.ewd', '.ewt', '.template', '.bak', '.d', '.lst', '.lin',
    '.gitignore', '.gitattributes', '.yml', '.yaml', '.toml', '.license',
    '.ino', '.a51', '.cpp', '.hpp', '.dbgconf', '.jlink', '.outjob',
    '.prjpcb', '.prjpcbstructure', '.url',
}

def problem(name, data):
    if data.startswith(ENCRYPTED_HEADER):
        return 'encrypted container header'
    if not data:
        return None
    suffix = pathlib.PurePosixPath(name).suffix.lower()
    if suffix == '.pdf':
        if not data.startswith(b'%PDF-'):
            return 'invalid PDF header'
        if re.search(rb'/Encrypt\b', data):
            return 'PDF encryption dictionary present'
        return None
    if suffix in {'.zip', '.docx', '.xlsx', '.pptx', '.pack'}:
        try:
            with zipfile.ZipFile(io.BytesIO(data)) as archive:
                for member in archive.infolist():
                    if member.flag_bits & 1:
                        return 'password-protected ZIP member'
                    if not member.is_dir():
                        nested = archive.read(member)
                        if nested.startswith(ENCRYPTED_HEADER):
                            return 'encrypted container inside ZIP'
                if archive.testzip():
                    return 'ZIP CRC check failed'
        except Exception:
            return 'unreadable ZIP container'
        return None
    signatures = {
        '.png': b'\x89PNG\r\n\x1a\n', '.jpg': b'\xff\xd8\xff',
        '.jpeg': b'\xff\xd8\xff', '.gif': b'GIF8', '.bmp': b'BM',
        '.schdoc': bytes.fromhex('d0cf11e0a1b11ae1'),
        '.pcbdoc': bytes.fromhex('d0cf11e0a1b11ae1'),
        '.xls': bytes.fromhex('d0cf11e0a1b11ae1'),
        '.doc': bytes.fromhex('d0cf11e0a1b11ae1'),
    }
    if suffix in signatures:
        return None if data.startswith(signatures[suffix]) else 'invalid format header'
    if (suffix in TEXT_EXTENSIONS or pathlib.PurePosixPath(name).name.lower() in
            {'license', 'copying', 'notice', 'makefile', '.gitignore', '.gitattributes'}):
        for encoding in ('utf-8-sig', 'gb18030', 'utf-16'):
            try:
                decoded = data.decode(encoding)
                controls = sum(ord(c) < 32 and c not in '\r\n\t\f' for c in decoded)
                if controls == 0:
                    return None
            except UnicodeError:
                pass
        return 'text could not be validated'
    return 'format not validated for publication'

def git_blob_items(revision=None):
    # Batch-read actual Git objects, rather than trusting working-tree reads.
    command = (['git', 'ls-tree', '-rz', '--full-tree', revision] if revision else
               ['git', 'ls-files', '--stage', '-z'])
    records = subprocess.check_output(command).split(b'\0')
    items = []
    for record in records:
        if not record:
            continue
        meta, name = record.split(b'\t', 1)
        fields = meta.split()
        oid = fields[2] if revision else fields[1]
        items.append((name.decode('utf-8'), oid))
    proc = subprocess.run(['git', 'cat-file', '--batch'],
                          input=b''.join(oid + b'\n' for _, oid in items),
                          stdout=subprocess.PIPE, check=True)
    offset = 0
    for name, oid in items:
        end = proc.stdout.index(b'\n', offset)
        header = proc.stdout[offset:end].split()
        if header[1] != b'blob':
            raise RuntimeError('Expected Git blob')
        size = int(header[2])
        start = end + 1
        yield name, proc.stdout[start:start+size]
        offset = start + size + 1

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--revision', help='check a commit/tree; default: staged index')
    args = parser.parse_args()
    failures = []
    count = total = 0
    for name, data in git_blob_items(args.revision):
        count += 1
        total += len(data)
        reason = problem(name, data)
        if reason:
            failures.append((name, reason))
    for name, reason in failures:
        print(f'BLOCKED: {name}: {reason}')
    print(f'Checked {count} Git blobs, {total} bytes; blocked={len(failures)}')
    return bool(failures)

if __name__ == '__main__':
    sys.exit(main())
