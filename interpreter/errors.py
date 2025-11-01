class VzoelError(Exception):
    """Base class for all Vzoel errors."""
    def __init__(self, message, line=None, column=None):
        self.message = message
        self.line = line
        self.column = column
        super().__init__(self.message)

    def __str__(self):
        if self.line:
            return f"[Baris {self.line}] Error: {self.message}"
        return f"Error: {self.message}"

class LexerError(VzoelError):
    """Custom error for lexer issues."""
    pass

class ParserError(VzoelError):
    """Custom error for parser issues."""
    pass

class VzoelRuntimeException(VzoelError):
    """Custom error for Vzoel runtime issues."""
    def __init__(self, token, message):
        self.token = token
        self.message = message
        # Make it robust for when a token is not available
        line = token.line if token else None
        super().__init__(message, line=line)

    def __str__(self):
        line_info = f"[Baris {self.token.line}] " if self.token else ""
        return f"{line_info}RuntimeError: {self.message}"

class VzoelModuleNotFound(VzoelRuntimeException):
    pass

class Return(VzoelRuntimeException):
    def __init__(self, value, token=None):
        # We don't really have a token for return statements, so we can pass None.
        self.value = value
        # The message for Return is not really used.
        super().__init__(token, "Return statement.")
