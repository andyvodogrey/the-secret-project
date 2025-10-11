# the-secret-project
First Embedded project



## First time clone repo
1. Clone the repo
2. Homebrew (macOS): 

```
    brew install pre-commit
    cd /path/to/repo
    pre-commit install
    pre-commit run --all-files

```
3. Windows Virtualenv
```
    cd /path/to/repo
    python3 -m venv .venv
    source .venv/bin/activate
    pip install pre-commit
    pre-commit install
```   

### Clang-Format-Style 
The format style based on LLVM, for more info check the link bellow 
* https://clang.llvm.org/docs/ClangFormatStyleOptions.html