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

    def test_missing_file(self):
        result = subprocess.run(
            [str(ANALYZER), "does_not_exist.bin"],
            capture_output=True,
            text=True
        )
        self.assertEqual(result.returncode, 2)
        self.assertIn("cannot open", result.stderr)

    def test_no_arguments(self):
        result = subprocess.run(
            [str(ANALYZER)],
            capture_output=True,
            text=True
        )
        self.assertEqual(result.returncode, 2)
        self.assertIn("usage", result.stderr)

    def test_directory_input(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            result = subprocess.run(
                [str(ANALYZER), tmpdir],
                capture_output=True,
                text=True
            )
            self.assertEqual(result.returncode, 2)
            self.assertIn("cannot read", result.stderr)

    def test_63_byte_header(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            test_file = Path(tmpdir) / "short63.bin"
            test_file.write_bytes(ANALYZER.read_bytes()[:63])

            result = subprocess.run(
                [str(ANALYZER), str(test_file)],
                capture_output=True,
                text=True
            )
            self.assertEqual(result.returncode, 1)
            self.assertIn("too short (63 bytes)", result.stderr)



    def test_magic_mismatch(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            test_file = Path(tmpdir) / "not_elf.bin"
            test_file.write_bytes(b"A" * 64)

            result = subprocess.run(
                [str(ANALYZER), str(test_file)],
                capture_output=True,
                text=True
            )

            self.assertEqual(result.returncode, 1)
            self.assertIn("mismatch", result.stderr)