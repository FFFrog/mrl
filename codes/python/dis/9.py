def f_gloabl():
    return None


def outer():
    V = 1
    def inner():
        return None

    inner()
    f_global()
