# send_audio_rtt.py

import pylink
import time
import os

RTT_BUFFER_INDEX = 0
AUDIO_FILE = 'alarm.raw'
CHUNK_SIZE = 256

def main():
    if not os.path.exists(AUDIO_FILE):
        print(f"Audio file {AUDIO_FILE} not found.")
        return

    jlink = pylink.JLink()
    jlink.open()
    jlink.connect('NRF52833_XXAA', verbose=True)

    jlink.rtt_start()
    time.sleep(0.1)

    with open(AUDIO_FILE, 'rb') as f:
        total_bytes_sent = 0

        while True:
            data = f.read(CHUNK_SIZE)
            if not data:
                break

            bytes_written = jlink.rtt_write(RTT_BUFFER_INDEX, data)
            total_bytes_sent += bytes_written
            time.sleep(0.01)

        # Send end-of-data signal
        jlink.rtt_write(RTT_BUFFER_INDEX, b'END')

    print(f"Audio upload completed. Total bytes sent: {total_bytes_sent}")
    jlink.close()

if __name__ == '__main__':
    main()
