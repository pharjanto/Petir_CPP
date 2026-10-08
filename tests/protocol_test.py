#!/usr/bin/env python3
"""End-to-end protocol tests using only Python's standard library."""
import queue, re, subprocess, sys, threading, time

class Engine:
    def __init__(self):
        self.p=subprocess.Popen([sys.argv[1]],stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,bufsize=1)
        self.lines=queue.Queue();self.transcript=[]
        def read():
            for line in self.p.stdout:self.lines.put(line.rstrip())
        self.reader=threading.Thread(target=read,daemon=True);self.reader.start()
    def send(self,s):self.p.stdin.write(s+'\n');self.p.stdin.flush()
    def until(self,prefix,timeout=10):
        end=time.monotonic()+timeout
        while time.monotonic()<end:
            try:line=self.lines.get(timeout=max(.001,end-time.monotonic()))
            except queue.Empty:break
            self.transcript.append(line)
            assert 'search stopped:' not in line and 'Error (search)' not in line,line
            assert not line.startswith(('info string ', 'Error:', 'Illegal move:')),line
            if line.startswith(prefix):return line
        raise AssertionError('Timed out waiting for '+prefix+'\n'+'\n'.join(self.transcript[-25:]))
    def close(self):
        if self.p.poll() is None:self.send('quit');self.p.wait(timeout=3)
        assert self.p.returncode==0,'Engine exited with '+str(self.p.returncode)
    def __enter__(self):return self
    def __exit__(self,*_):self.close()

def best(e,timeout=10):
    line=e.until('bestmove ',timeout)
    assert re.fullmatch(r'bestmove (?:[a-h][1-8][a-h][1-8][qrbn]?|0000)',line),line
    return line.split()[1]

with Engine() as e:
    e.send('uci');e.until('uciok');e.send('isready');e.until('readyok')
    e.send('setoption name Hash value 16');e.send('setoption name OwnBook value false')
    e.send('position startpos');e.send('go depth 5');assert best(e)!='0000'
    e.send('position startpos moves e2e4 e7e5 g1f3');e.send('go depth 6');assert best(e)!='0000'
    e.send('position fen 7k/6Q1/6K1/8/8/8/8/8 b - - 0 1');e.send('go depth 5');assert best(e)=='0000'
    e.send('position fen 7k/8/8/3pP3/8/8/8/K7 w - d6 42 17');e.send('go movetime 50');assert best(e,2)!='0000'
    e.send('position startpos');e.send('go infinite');e.send('isready');e.until('readyok',2)
    time.sleep(.03);start=time.monotonic();e.send('stop');assert best(e,2)!='0000';assert time.monotonic()-start<2
    e.send('position startpos');e.send('go ponder wtime 30000 btime 30000');time.sleep(.03);e.send('ponderhit');assert best(e,3)!='0000'
    e.send('position startpos');e.send('setoption name OwnBook value true');e.send('go depth 5');assert best(e)!='0000'

with Engine() as e:
    e.send('xboard');e.send('protover 2');assert 'setboard=1' in e.until('feature ')
    e.send('new');e.send('force');e.send('usermove e2e4');e.send('usermove e7e5');e.send('undo')
    e.send('ping 123');assert e.until('pong ')=='pong 123'
    e.send('sd 5');e.send('go');line=e.until('move ',5);assert re.fullmatch(r'move [a-h][1-8][a-h][1-8][qrbn]?',line),line
    e.send('force');e.send('setboard 7k/6Q1/6K1/8/8/8/8/8 b - - 0 1');e.send('go');assert 'Checkmate' in e.until('1-0 ',3)

print('UCI and WinBoard protocol tests passed')
