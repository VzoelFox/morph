class VzoelRuntimeException(RuntimeError):
    """Error kustom untuk masalah saat runtime Vzoel."""
    def __init__(self, token, message):
        self.token = token
        self.message = message
        super().__init__(message)

class VzoelModuleNotFound(VzoelRuntimeException):
    """Error khusus untuk file modul yang tidak ditemukan."""
    pass

class Return(VzoelRuntimeException):
    """Error khusus untuk return statement."""
    def __init__(self, value):
        super().__init__(None, "Return")
        self.value = value
