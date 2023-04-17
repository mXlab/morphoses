class Chrono:
    def __init__(self, world):
        self.world = world
        self.start_time = 0
        self.started = False

    def start(self):
        self.start_time = self.world.get_time()
        self.started = True

    def elapsed(self):
        return self.world.get_time() - self.start_time

    def has_passed(self, t):
        return self.started and self.elapsed() >= t

    def is_started(self):
        return self.started

    def stop(self):
        self.started = False