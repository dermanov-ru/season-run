import math,random,wave,array
from pathlib import Path
rate=32000
bpm=116
beat=60/bpm
length=32*beat
count=int(length*rate)
rng=random.Random(29)
samples=array.array('f')
roots=[38,34,41,36]
notes=[0,7,12,15,12,7,10,7]
for i in range(count):
 t=i/rate; b=t/beat; phase=b%1; bar=int(b//4); root=roots[(bar//2)%4]
 hz=440*2**((root-69)/12)
 bass=.17*math.sin(2*math.pi*hz*t)*math.exp(-phase*5)
 eighth=(b*2)%1; note=root+24+notes[int(b*2)%8]
 f=440*2**((note-69)/12)
 arp=.095*(math.sin(2*math.pi*f*t)+.25*math.sin(4*math.pi*f*t))*math.exp(-eighth*4)
 pad=0
 for semitone in [0,7,12]:
  pf=440*2**((root+12+semitone-69)/12)
  pad+=.027*math.sin(2*math.pi*pf*t)*(.65+.35*math.sin(math.pi*(b%8)/8)**2)
 sec=phase*beat
 kick=.28*math.sin(2*math.pi*(45*sec+12*(1-math.exp(-sec*35))))*math.exp(-sec*22)
 noise=rng.uniform(-1,1)
 snare=.11*noise*math.exp(-sec*27) if int(b)%4 in [1,3] else 0
 hat=.035*noise*math.exp(-eighth*30)
 fade=min(1,t/.04,(length-t)/.04)
 samples.append((bass+arp+pad+kick+snare+hat)*max(0,fade))
peak=max(abs(v) for v in samples)
out=array.array('h')
for i,v in enumerate(samples):
 out.append(int(v/peak*24000))
 delayed=samples[(i-73)%count]
 out.append(int((v*.85+delayed*.15)/peak*24000))
p=Path(str(Path(__file__).resolve().parents[1] / 'Audio/WinterPulse.wav'))
with wave.open(str(p),'wb') as w:
 w.setnchannels(2);w.setsampwidth(2);w.setframerate(rate);w.writeframes(out.tobytes())
print('Original loop:',round(length,2),'seconds')
