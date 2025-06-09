import pygame
import serial
import struct
import time

# === CONFIGURAZIONE ===
SERIAL_PORT = 'COM5'      # <-- Cambia questo con la porta corretta
BAUD_RATE = 115200
DELAY_MS = 50             # invio ogni 50ms

# === INIZIALIZZAZIONE SERIAL E JOYSTICK ===
ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() == 0:
    raise Exception("Nessun joystick trovato")
joystick = pygame.joystick.Joystick(0)
joystick.init()

print(f"Joystick '{joystick.get_name()}' con {joystick.get_numaxes()} assi")

def map_axis(val):
    """
    Mappa da valore [-1.0, +1.0] a [0, 80]
    """
    return max(0, min(80, int(round((val + 1.0) * 40.0))))

try:
    while True:
        pygame.event.pump()  # Aggiorna gli eventi joystick

        # Leggi asse verticale sinistro (asse 1) e destro (asse 4)
        left_raw = joystick.get_axis(1)   # stick sinistro verticale
        right_raw = joystick.get_axis(4)  # stick destro verticale

        # Mappatura a [0,80]
        left_mapped = map_axis(left_raw)
        right_mapped = map_axis(right_raw)

        # Debug stampato a schermo
        print(f"Left: {left_mapped:3} | Right: {right_mapped:3}")

        # Prepara pacchetto: 0xAA + 2 float little endian
        packet = b'\xAA' + struct.pack('<ff', float(left_mapped), float(right_mapped))
        ser.write(packet)

        time.sleep(DELAY_MS / 1000.0)

except KeyboardInterrupt:
    print("Terminato da utente")

finally:
    ser.close()
    pygame.quit()
