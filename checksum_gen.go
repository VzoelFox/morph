package main

import (
	"crypto/sha256"
	"fmt"
	"io"
	"os"
	"path/filepath"
	"strings"
)

const (
	checksumDir = ".morph.vz/checksums"
	rootDir     = "."
)

func main() {
	err := filepath.Walk(rootDir, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}

		if info.IsDir() {
			// Skip .git
			if info.Name() == ".git" {
				return filepath.SkipDir
			}
			// Skip .morph.vz but NOT .vzoel.jules
			if info.Name() == ".morph.vz" {
				// But wait, .morph.vz/context might need checksums?
				// User said "archive context".
				// But existing checksums/updates.sha256 exist?
				// Let's assume we skip .morph.vz/checksums but scan .morph.vz/context?
				// But if we write to .morph.vz/checksums, we shouldn't scan it.
				return nil // Don't skip dir yet, check subdirs
			}
			return nil
		}

        // Skip files in .git
        if strings.Contains(path, ".git/") {
            return nil
        }

        // Skip files in .morph.vz/checksums
        if strings.Contains(path, ".morph.vz/checksums") {
            return nil
        }

		// Skip self
		if info.Name() == "checksum_gen.go" {
			return nil
		}
        // Skip compiled binaries
        if info.Name() == "jules" || strings.HasSuffix(info.Name(), ".exe") {
             return nil
        }

		return generateChecksum(path)
	})

	if err != nil {
		fmt.Printf("Error walking path: %v\n", err)
		os.Exit(1)
	}

    fmt.Println("Checksum generation complete.")
}

func generateChecksum(path string) error {
    f, err := os.Open(path)
    if err != nil {
        return err
    }
    defer f.Close()

    h := sha256.New()
    if _, err := io.Copy(h, f); err != nil {
        return err
    }

    hash := fmt.Sprintf("%x", h.Sum(nil))

    // Output format: hash  filename
    // filename should be relative to root
    cleanPath := filepath.Clean(path)
    if strings.HasPrefix(cleanPath, "./") {
        cleanPath = cleanPath[2:]
    }

    content := fmt.Sprintf("%s  %s\n", hash, cleanPath)

    // Target file
    // If path is "pkg/lexer/token.go", target is ".morph.vz/checksums/pkg/lexer/token.go.sha256"
    targetPath := filepath.Join(checksumDir, cleanPath+".sha256")

    if err := os.MkdirAll(filepath.Dir(targetPath), 0755); err != nil {
        return err
    }

    if err := os.WriteFile(targetPath, []byte(content), 0644); err != nil {
        return err
    }

    fmt.Printf("Generated checksum for %s\n", cleanPath)
    return nil
}
