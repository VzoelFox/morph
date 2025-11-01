class VzoelRuntimeException(RuntimeError):
    """Error kustom untuk masalah saat runtime Vzoel."""
    def __init__(self, token, message):
        self.token = token
        self.message = message
        super().__init__(message)

class VzoelModuleNotFound(VzoelRuntimeException):
    pass

class Return(VzoelRuntimeException):
    def __init__(self, value):
        self.value = value
    """Error khusus untuk file modul yang tidak ditemukan."""
    pass
