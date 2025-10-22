class VzoelRuntimeException(Exception):
    def __init__(self, token, message):
        self.token = token
        self.message = message
        super().__init__(message)

class Return(VzoelRuntimeException):
    def __init__(self, value):
        self.value = value
