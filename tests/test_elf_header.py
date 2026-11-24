from pathlib import Path
import subprocess
import unittest
import tempfile
import os

PROJECT_ROOT = Path(__file__).resolve().parent.parent
ANALYZER = Path(os.environ["ELF_ANALYZER"])

class ElfHeaderTests(unittest.TestCase):
    def test_valid_elf64(self):
        result = subprocess.run(
            [str(ANALYZER), str(ANALYZER)],
            capture_output=True,
            text=True
        )
        self.assertEqual(result.returncode, 0)
        self.assertIn("Class: ELF64", result.stdout)

    def test_invalid_headers(self):
        cases = [
            ("elf32", 4, 0x01, "unsupported: ELF32"),
            ("invalid_class", 4, 0x03, "invalid: unknown ELF class"),
            ("big_endian", 5, 0x02, "unsupported: big-endian ELF"),
            ("invalid_endian", 5, 0x03, "invalid: unknown ELF endian"),
            ("ident_version", 6, 0x02, "invalid: unsupported ELF identification version"),
            ("header_version", 20, 0x02, "invalid: unexpected ELF header version"),
            ("header_size", 52, 0x3F, "invalid: unexpected ELF64 header size"),
        ]

        original = ANALYZER.read_bytes()

        with tempfile.TemporaryDirectory() as tmpdir:
            for name, offset, value, expected in cases:
                with self.subTest(case=name):
                    data = bytearray(original)
                    data[offset] = value

                    test_file = Path(tmpdir) / f"{name}.bin"
                    test_file.write_bytes(data)

                    result = subprocess.run(
                        [str(ANALYZER), str(test_file)],
                        capture_output=True,
                        text=True
                    )

                    self.assertEqual(result.returncode, 1)
                    self.assertIn(expected, result.stderr)

    def test_truncated_header(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            test_file = Path(tmpdir) / "truncated.bin"
            test_file.write_bytes(b"\x7fELF")

            result = subprocess.run(
                [str(ANALYZER), str(test_file)],
                capture_output=True,
                text=True
            )

            self.assertEqual(result.returncode, 1)
            self.assertIn("too short", result.stderr)
            self.assertNotIn("ELF Header:", result.stdout)