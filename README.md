# Morph Programming Language

🚀 **Self-Hosted Programming Language** - Morph has achieved full self-hosting! The Go compiler has been retired and Morph now compiles itself.

## Quick Start

```bash
# Run interpreter mode
./morph program.fox

# Compile to native C binary  
./morph build program.fox

# Use self-hosted MorphSH compiler
cd morphsh && ../morph main.fox

# Package management
./morph_package_manager install <package>
```

## Architecture Status

- ✅ **N1**: Current stable compiler (C transpilation) - **PRODUCTION READY**
- 🚧 **N3**: Pure Morph compilation target (in development)  
- 📋 **N4**: FoxVM bytecode target (planned)

## Self-Hosted Ecosystem

- **MorphSH**: Complete self-hosted compiler written in Morph
- **Package Manager**: Git-style package management system
- **Runner System**: Command execution framework
- **Standard Library**: 32 complete modules (Core, System, I/O, Concurrency)

This represents a major milestone - Morph is now completely independent from Go!