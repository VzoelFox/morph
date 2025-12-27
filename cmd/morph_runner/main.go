package main

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"
)

func main() {
	if len(os.Args) < 2 {
		printUsage()
		return
	}

	baseDir, err := resolveBaseDir()
	if err != nil {
		fmt.Printf("Error resolving runner path: %v\n", err)
		os.Exit(1)
	}

	switch os.Args[1] {
	case "to":
		if len(os.Args) < 3 {
			fmt.Println("Usage: morph to <file.fox>")
			os.Exit(1)
		}
		fmt.Printf("🚀 Running: %s\n", os.Args[2])
		runCommand(filepath.Join(baseDir, "morphsh", "main"), os.Args[2])
	case "spawn":
		if len(os.Args) < 3 {
			fmt.Println("Usage: morph spawn <file.fall>")
			os.Exit(1)
		}
		fmt.Printf("⭐ Spawning Fall script: %s\n", os.Args[2])
		runCommand(filepath.Join(baseDir, "star_spawn"), os.Args[2])
	case "install":
		if len(os.Args) < 3 {
			fmt.Println("Usage: morph install <package>")
			os.Exit(1)
		}
		fmt.Printf("📦 Installing package: %s\n", os.Args[2])
		runCommand(filepath.Join(baseDir, "morph_package_manager"), "install", os.Args[2])
	case "list":
		runCommand(filepath.Join(baseDir, "morph_package_manager"), "list")
	default:
		printUsage()
	}
}

func printUsage() {
	fmt.Println("Morph Self-Hosted Runner")
	fmt.Println("Usage:")
	fmt.Println("  morph to <file.fox>      # Run Morph file")
	fmt.Println("  morph spawn <file.fall>  # Run Fall script")
	fmt.Println("  morph install <package>  # Install package")
	fmt.Println("  morph list               # List packages")
}

func runCommand(name string, args ...string) {
	cmd := exec.Command(name, args...)
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr
	cmd.Stdin = os.Stdin
	if err := cmd.Run(); err != nil {
		fmt.Printf("Command failed: %v\n", err)
		os.Exit(1)
	}
}

func resolveBaseDir() (string, error) {
	if base := os.Getenv("MORPH_HOME"); base != "" {
		return base, nil
	}
	return os.Getwd()
}
