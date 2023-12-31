import websocket
try:
    import thread
except ImportError:
    import _thread as thread
import time

def on_message(ws, message):
    print(message)

def on_error(ws, error):
    print(error)

def on_close(ws):
    print("### closed ###")

def on_ping(ws, data):
    pass
    #print("rx ping")

def on_pong(ws, data):
    pass
    #print("rx pong")



def on_open(ws):
    ws.send('{"t": "sub","p": {"i": "BattAmps"}}')
    def run(*args):
        while True:
            time.sleep(100)
        ws.close()
    thread.start_new_thread(run, ())


if __name__ == "__main__":
    websocket.enableTrace(False)
    ws = websocket.WebSocketApp("ws://172.20.20.153/ws",
                              on_open = on_open,
                              on_message = on_message,
                              on_error = on_error,
                              on_close = on_close,
                              on_ping = on_ping,
                              on_pong = on_pong)
    ws.run_forever(ping_interval=40)
