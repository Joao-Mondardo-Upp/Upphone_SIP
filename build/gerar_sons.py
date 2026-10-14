# Gera os sons padrao do Upphone SIP (res/*.wav), embutidos no Upphone.exe (RCDATA em res/main.rc2).
# Sons proprios, sinteticos: nao dependem dos arquivos do MicroSIP. Tons de 425 Hz seguem o padrao brasileiro.
# Uso: python build/gerar_sons.py   (grava em res/)
import math
import os
import struct
import wave

RATE = 16000
RES = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "res")


def silence(seconds):
    return [0.0] * int(RATE * seconds)


def tone(freqs, seconds, volume, fade=0.01, trill=0.0):
    """Tom com envelope (fade de entrada e saida). trill > 0 alterna entre as frequencias nesse periodo."""
    n = int(RATE * seconds)
    out = []
    for i in range(n):
        t = i / RATE
        if trill:
            f = freqs[int(t / trill) % len(freqs)]
            s = math.sin(2 * math.pi * f * t) + 0.25 * math.sin(2 * math.pi * 2 * f * t)
            s /= 1.25
        else:
            s = sum(math.sin(2 * math.pi * f * t) for f in freqs) / len(freqs)
        env = min(1.0, t / fade, (seconds - t) / fade)
        out.append(s * env * volume)
    return out


def save(name, samples):
    path = os.path.join(RES, name)
    with wave.open(path, "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(b"".join(struct.pack("<h", int(max(-1.0, min(1.0, s)) * 32767)) for s in samples))
    print("%s: %.2f s" % (path, len(samples) / RATE))


# Chamada recebida (toca em repeticao): "trim-trim" de telefone, duas rajadas e pausa (ciclo de 3,5 s)
burst = tone([784, 988], 0.5, 0.55, fade=0.015, trill=0.04)
save("ringtone.wav", burst + silence(0.2) + burst + silence(2.3))

# Chamando (o app repete a cada 4,5 s): 425 Hz por 1 s, como o sinal de chamada brasileiro
save("ringing.wav", tone([425], 1.0, 0.3))

# Segunda chamada durante outra / atendimento automatico: dois bipes curtos de 425 Hz
beep = tone([425], 0.12, 0.35, fade=0.008)
save("ringing2.wav", beep + silence(0.12) + beep)

# Fim de chamada: tres bipes descendentes
notes = []
for f in (660, 550, 440):
    notes += tone([f], 0.1, 0.35, fade=0.008) + silence(0.05)
save("hangup.wav", notes)
