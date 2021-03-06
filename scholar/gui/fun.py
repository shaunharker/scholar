import asyncio


class Fun:
    def __init__(self, f, *args, output=None, aux=None, **kwargs):
        self.args = args
        if output is None:
            self.output = []
        else:
            self.output = output
        self.aux = aux # anything we want to keep a reference to so it doesn't get gc'ed
        self.kwargs = kwargs
        self.task = asyncio.create_task(Fun.loop(f, args, kwargs, self.output))

    def __del__(self):
        self.task.cancel()

    @staticmethod
    async def loop(f, args, kwargs, output):
        # print(f"Fun.loop(args={args},kwargs={kwargs},output={output})")
        args_at_step = lambda n: [x[n] for x in args]
        kwargs_at_step = lambda n: {k:v[n] for (k,v) in kwargs.items()}
        closure = lambda n: f(*args_at_step(n), **kwargs_at_step(n))
        n = len(output)
        while True:
            try:
                output.append(closure(n))
                n += 1
            except asyncio.CancelledError:
                return
            except:
                await asyncio.sleep(.1)
