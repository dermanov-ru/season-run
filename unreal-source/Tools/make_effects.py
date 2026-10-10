import wave,math,array,random
from pathlib import Path
rate=32000
root=Path(str(Path(__file__).resolve().parents[1] / 'Audio'))
for name,duration in [('CoinChime',.18),('TurboWhoosh',.65)]:
 pcm=array.array('h');rng=random.Random(12);filtered=0
 for i in range(int(rate*duration)):
  t=i/rate;p=t/duration
  if name=='CoinChime':
   envelope=min(1,t/.004)*math.exp(-t*24)*(1-p)
   v=envelope*(.55*math.sin(2*math.pi*1318.51*t)+.25*math.sin(2*math.pi*1977.765*t)+.12*math.sin(2*math.pi*2637.02*t))
  else:
   noise=rng.uniform(-1,1);filtered=filtered*.87+noise*.13
   env=math.sin(math.pi*p)**1.4
   phase=2*math.pi*(170*t+500*t*t)
   v=env*(filtered*.95+.12*math.sin(phase))
  sample=int(max(-1,min(1,v))*28000);pcm.extend([sample,sample])
 with wave.open(str(root/(name+'.wav')),'wb') as w:
  w.setnchannels(2);w.setsampwidth(2);w.setframerate(rate);w.writeframes(pcm.tobytes())
