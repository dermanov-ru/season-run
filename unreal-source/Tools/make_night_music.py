import math,random,array,wave
from pathlib import Path
rate=32000;beat=60/142;length=32*beat;rng=random.Random(66);data=array.array('f')
roots=[38,41,34,36];melody=[0,7,12,14,15,12,7,10,0,7,12,19,17,15,14,7]
for i in range(int(length*rate)):
 t=i/rate;b=t/beat;bar=int(b/4);root=roots[(bar//2)%4];phase=b%1;step=int(b*2);ep=(b*2)%1
 f=440*2**((root+24+melody[step%16]-69)/12);env=min(1,ep*25)*math.exp(-ep*2.8)
 lead=sum(math.sin(2*math.pi*f*t*k)/k for k in [1,2,3,4])*.09*env
 bassf=440*2**((root-69)/12);bass=.18*math.sin(2*math.pi*bassf*t)*math.exp(-phase*6)
 sec=phase*beat;kick=.27*math.sin(2*math.pi*(44*sec+9*(1-math.exp(-sec*48))))*math.exp(-sec*25)
 noise=rng.uniform(-1,1);clap=.10*noise*math.exp(-sec*28) if int(b)%4 in [1,3] else 0
 hat=.035*noise*math.exp(-((b*4)%1)*24)
 pad=sum(.027*math.sin(2*math.pi*440*2**((root+12+n-69)/12)*t) for n in [0,7,12])*(.3+.7*phase)
 fade=min(1,t/.02,(length-t)/.02);data.append((lead+bass+kick+clap+hat+pad)*max(0,fade))
peak=max(abs(x) for x in data);pcm=array.array('h')
for i,x in enumerate(data):pcm.extend([int(x/peak*24000),int((x*.82+data[(i-99)%len(data)]*.18)/peak*24000)])
with wave.open(str(Path(__file__).resolve().parents[1] / 'Audio/NightFlight.wav'),'wb') as w:w.setnchannels(2);w.setsampwidth(2);w.setframerate(rate);w.writeframes(pcm.tobytes())
