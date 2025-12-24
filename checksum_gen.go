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

        // Skip binaries and artifacts
        if info.Name() == "jules" || strings.HasSuffix(info.Name(), ".exe") {
             return nil
        }

        // Skip C/H/O artifacts unless they are source templates in pkg/compiler/runtime
        ext := filepath.Ext(info.Name())
        if ext == ".o" || ext == ".a" || ext == ".so" || ext == ".dylib" || ext == ".test" {
            return nil
        }

        // Skip .c and .h files unless they are templates (ending in .tpl) or inside pkg/compiler/runtime (source)
        // Actually runtime.c is renamed to runtime.c.tpl in repo.
        // So we should generally skip .c and .h unless specific exceptions?
        // Let's rely on extension check.
        if (ext == ".c" || ext == ".h") && !strings.HasSuffix(path, ".tpl") {
            // Check if it's explicitly allowed source?
            // Currently Morph uses .tpl for embedded C source.
            // So any raw .c/.h file is likely an artifact.
            return nil
        }

        // Skip binary files without extension in examples/
        if strings.Contains(path, "examples/") && ext == "" {
            // Check if executable bit is set? Hard in portable Go.
            // Just skip common binary names or everything without ext in examples/ except known ones?
            // .fox files have extension.
            // So scanning examples/ without extension is likely binaries.
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
