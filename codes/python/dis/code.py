g_value = 1


class Code:
    def __init__(self):
        super().__init__()

    def outer(self, local_arg1, local_closure1):
        global g_value
        g_value = 10

        local_arg2 = 1
        local_colsure2 = local_arg1 + local_arg2

        def inner():
            return local_closure1 + local_colsure2

        return inner
