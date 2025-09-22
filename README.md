# MK2 CAN Controller

Questo progetto permette di inviare comandi ai motori del robot **MK2** tramite **CAN bus** in due modalità:  

- **Modalità automatica (random)**: se la variabile `automaticSend` è impostata a `true`, il programma invia periodicamente valori randomici ai motori. In questo caso il modulo sender può funzionare in maniera autonoma, senza essere collegato al PC.  
- **Modalità joystick (seriale)**: se `automaticSend` è impostata a `false`, il programma attende dati provenienti dalla seriale. I dati vengono inviati da un **joystick PlayStation 4**, convertiti in due valori di velocità (sinistra e destra) e trasmessi via CAN ai motori.  

---

## ⚙️ Requisiti  

### Hardware  
- **Raspberry Pi Pico**  
- **Modulo MCP2515** per la comunicazione CAN  
- **Robot MK2** con motori compatibili CAN  
- **Joystick PlayStation 4** (per il controllo manuale, via seriale)  
- PC con porta seriale disponibile (per la modalità joystick)  

### Software / Librerie  
- **Arduino IDE** (con supporto per Raspberry Pi Pico)  
- **Python 3** con libreria `pygame`  
- Librerie Arduino incluse nel progetto:  
  - `mcp2515.h` (driver CAN)  
  - `CanWrapper.h` (gestione CAN custom)  
  - `Dynamixel_ll.h` e altre librerie personalizzate  

---

## 🚀 Installazione  

### Firmware Raspberry Pi Pico  
1. Clonare o scaricare questo repository.  
2. Aprire il progetto nell’**VSCODE**.
3. Compilare attraverso il makefile con il comando **make compile** e scaricare sul pico con **make upload bootsel** dopo aver collegato il pico che trasemtte al modulo in modalità bootsel al pc


### Script Python per joystick PS4  
1. Installare Python 3 e `pygame`:  
   ```
   pip install pygame
   ```
2. Modificare la variabile SERIAL_PORT nello script Python con la porta corretta del Pico (es. COM5 o /dev/ttyACM0).
3. Eseguire lo script Python:
   ```
   python ps4_serial_sender.py
   ```

---

## ▶️ Utilizzo

### Modalità automatica (random)
Impostare nel codice Arduino:
```
bool automaticSend = true;
```
Il Pico invierà valori randomici (ogni 2.5 secondi viene ricalcolato un nuovo valore) a entrambi i motori tramite CAN.

Non è necessario avere il PC collegato.

### Modalità joystick (seriale)
Impostare nel codice Arduino:
```
bool automaticSend = false;
```
Collegare il Pico al PC.

Eseguire lo script Python per leggere il joystick PS4.

I due valori di velocità vengono impacchettati e trasmessi via CAN ai motori del robot.

---

## 📡 Protocollo di comunicazione
I messaggi CAN vengono inviati con ID MOTOR_SETPOINT.

Ogni messaggio contiene 2 float (8 byte) che rappresentano le velocità del motore sinistro e destro.

In modalità seriale, il pacchetto inviato dalla Python app ha il formato:
```
0xAA [4 byte float vel_left] [4 byte float vel_right]
```

---

## 📖 Note

- In modalità random, i valori generati sono compresi tra 0 e 50.
- In modalità joystick, i valori degli assi verticali degli stick PS4 sono mappati da [-1, 1] a [0, 80].
- La frequenza di invio seriale è regolabile tramite la variabile DELAY_MS nello script Python.

---

## 📊 Diagramma a blocchi

**Modalità joystick (seriale)**
```
[Joystick PS4] 
      │
      ▼
   [PC Python Script] 
      │ Serial (0xAA + 2 float)
      ▼
[Raspberry Pi Pico Firmware] 
      │ CAN (ID: MOTOR_SETPOINT)
      ▼
[Raspberry Pi Pico modulo MK2]
```

**Modalità automatica (random)**
```
[Raspberry Pi Pico Firmware] 
      │ genera valori random
      │ CAN (ID: MOTOR_SETPOINT)
      ▼
[Raspberry Pi Pico modulo MK2]
```
```



