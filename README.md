# Sysmon Package

A system monitoring application for STM32MP157F-DK2 embedded Linux system.

## Overview

Sysmon is a buildroot package that provides system monitoring capabilities for embedded Linux systems. It includes framebuffer drawing utilities for display output.

## Files

- `Config.in` - Buildroot package configuration
- `sysmon.mk` - Buildroot package makefile
- `src/sysmon.c` - Main application source code
- `src/Makefile` - Source code makefile

## Building

This package is designed to be built with buildroot for ARM cross-compilation:

1. Copy the sysmon directory to your buildroot-external package directory
2. Add the package to your buildroot configuration
3. Build with: `make sysmon`

## Usage

After building and installing to the target system, run:

```bash
sysmon
```

## Target System

- **Platform**: STM32MP157F-DK2
- **Architecture**: ARM (32-bit)
- **OS**: Embedded Linux (buildroot)

## License

MIT License
