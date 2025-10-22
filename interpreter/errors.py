class VzoelRuntimeException(RuntimeError):
    def __init__(self, token, message):
        self.token = token
        self.message = message
        super().__init__(message)

class VzoelModuleNotFound(VzoelRuntimeException):
    pass

class Return(VzoelRuntimeException):
    def __init__(self, value):
        self.value = value
