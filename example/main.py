import gi

gi.require_version("GCron", "0")

from datetime import datetime
from gi.repository import GCron, GLib


def callback(userdata):
    print(datetime.now(), userdata)
    return GLib.SOURCE_CONTINUE


context = GLib.MainContext()
source = GCron.source_new(" * * * * *")
source.set_callback(callback, "hello")

source.attach(context)
loop = GLib.MainLoop.new(context, True)
loop.run()
