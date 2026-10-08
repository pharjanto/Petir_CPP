#!/usr/bin/env python3
"""End-to-end protocol tests using only Python's standard library."""
import os, queue, re, subprocess, sys, threading, time

class Engine:
    def __init__(self):
        self.p=subprocess.Popen([sys.argv[1]],stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,bufsize=1)
        self.lines=queue.Queue();self.transcript=[]
        def read():
            for line in self.p.stdout:self.lines.put(line.rstrip())
        self.reader=threading.Thread(target=read,daemon=True);self.reader.start()
    def send(self,s):self.p.stdin.write(s+'\n');self.p.stdin.flush()
    def until(self,prefix,timeout=10,allow_error=False):
        end=time.monotonic()+timeout
        while time.monotonic()<end:
            try:line=self.lines.get(timeout=max(.001,end-time.monotonic()))
            except queue.Empty:break
            self.transcript.append(line)
            if not allow_error:
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

def barrier(e,number=789):
    e.send('ping '+str(number))
    assert e.until('pong ')==f'pong {number}'

def search_infos(lines):
    return [line for line in lines if line.startswith('info depth ')]

def assert_completed_best(e,move):
    infos=search_infos(e.transcript)
    assert infos and infos[-1].split(' pv ')[1].split()[0]==move,e.transcript[-10:]

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
    e.send('uci');e.until('uciok');e.send('setoption name OwnBook value false')
    for depth in (1,2,3,4,5):
        e.send('setoption name Clear Hash');e.send('position startpos')
        e.transcript.clear();e.send(f'go depth {depth} wtime 60000 btime 60000')
        move=best(e,3);infos=search_infos(e.transcript)
        assert int(infos[0].split()[2])==1,infos
        assert {int(line.split()[2]) for line in infos}==set(range(1,depth+1)),infos
        assert max(int(line.split()[2]) for line in infos)==depth,infos
        assert_completed_best(e,move)
    # The second search starts its depth loop anew but reuses the first search's
    # hash results, so it keeps the answer while visiting fewer nodes.
    for mtdf,depth in ((False,7),(True,4)):
        e.send('setoption name MTDf value '+str(mtdf).lower())
        e.send('setoption name Clear Hash')
        searches=[]
        for run in range(2):
            e.send('position startpos moves e2e4 e7e5 g1f3');e.transcript.clear()
            e.send(f'go depth {depth}');move=best(e,5);infos=search_infos(e.transcript)
            if run==0:
                assert infos and int(infos[0].split()[2])==1,infos
                assert {int(line.split()[2]) for line in infos}==set(range(1,depth+1)),infos
            assert int(infos[-1].split()[2])==depth,infos
            assert_completed_best(e,move)
            nodes=int(next(line.split()[2] for line in e.transcript if line.startswith('info nodes ')))
            searches.append((move,infos[-1].split()[5],nodes))
        assert searches[0][:2]==searches[1][:2] and searches[1][2]<searches[0][2],searches
    e.send('setoption name MTDf value false')
    for limit in ('movetime 0','wtime 0 btime 0','wtime 20 btime 20'):
        e.send('position startpos');start=time.monotonic();e.send('go '+limit)
        assert best(e,1)!='0000' and time.monotonic()-start<.5,limit
    for _ in range(3):
        e.send('position startpos');e.send('go infinite');time.sleep(.08)
        e.send('stop');assert_completed_best(e,best(e,2))
    # Replacing a position cancels the old search without a stale bestmove.
    e.send('position startpos');e.send('go infinite');time.sleep(.03)
    e.send('position fen 7k/6Q1/6K1/8/8/8/8/8 b - - 0 1');e.send('isready')
    e.transcript.clear();e.until('readyok',2)
    assert not any(line.startswith('bestmove ') for line in e.transcript),e.transcript
    e.send('go depth 1');assert best(e)=='0000'
    fen='r3k2r/8/8/3pP3/8/8/8/R3K2R w KQkq d6 42 17'
    e.send('position fen '+fen);e.send('d');assert e.until('r3k2r/')==fen
    e.send('position startpos moves e2e4 e7e9')
    e.until('info string ',allow_error=True);e.send('d');assert e.until('r3k2r/')==fen

with Engine() as e:
    e.send('xboard');e.send('protover 2');assert 'setboard=1' in e.until('feature ')
    e.send('new');e.send('force');e.send('usermove e2e4');e.send('usermove e7e5');e.send('undo')
    e.send('ping 123');assert e.until('pong ')=='pong 123'
    e.send('sd 5');e.send('go');line=e.until('move ',5);assert re.fullmatch(r'move [a-h][1-8][a-h][1-8][qrbn]?',line),line
    e.send('force');e.send('setboard 7k/6Q1/6K1/8/8/8/8/8 b - - 0 1');e.send('go');assert 'Checkmate' in e.until('1-0 ',3)

with Engine() as e:
    e.send('uci');e.until('uciok');e.send('setoption name OwnBook value false')
    e.send('xboard');e.send('new');e.send('force');e.send('sd 5')
    searches=[]
    for run in range(2):
        e.transcript.clear();e.send('go');move=e.until('move ',5)
        infos=[line.split() for line in e.transcript if re.match(r'^\d+ -?\d+ \d+ \d+ ',line)]
        if run==0:
            assert infos and int(infos[0][0])==1,infos
            assert {int(line[0]) for line in infos}==set(range(1,6)),infos
        assert int(infos[-1][0])==5,infos
        searches.append((move,infos[-1][1],int(infos[-1][3])))
        e.send('force');e.send('undo');barrier(e)
    assert searches[0][:2]==searches[1][:2] and searches[1][2]<searches[0][2],searches

with Engine() as e:
    e.send('uci');e.until('uciok');e.send('setoption name OwnBook value false')
    e.send('xboard');e.send('new');e.send('force');e.send('st 5');e.send('go')
    # None of these routine commands may stop the search or publish a move.
    time.sleep(.03);e.transcript.clear()
    for command in ('time 30000','otim 30000','nopost','post','hard','easy','accepted ping','name opponent','unsupported_command'):
        e.send(command)
    barrier(e)
    assert not any(line.startswith('move ') for line in e.transcript),e.transcript
    e.send('?');e.until('move ',2)
    e.send('force');e.send('new');e.send('force');e.send('st 5');e.send('go');time.sleep(.03)
    e.send('force');e.transcript.clear();barrier(e,790)
    assert not any(line.startswith('move ') for line in e.transcript),e.transcript
    # A discarded engine move must not change the board or side to move.
    e.send('usermove e2e4');e.send('sd 1');e.send('go');move=e.until('move ',2).split()[1]
    assert move[1] in '78',move
    for command in ('new','result 1/2-1/2','setboard 7k/6Q1/6K1/8/8/8/8/8 b - - 0 1'):
        e.send('force');e.send('new');e.send('st 5');e.send('go');time.sleep(.02)
        e.send(command);e.transcript.clear();barrier(e,791)
        assert not any(line.startswith('move ') for line in e.transcript),e.transcript
    # A late clock update still enforces the supplied remaining time.
    e.send('force');e.send('new');e.send('level 0 3 0');e.send('go');time.sleep(.02)
    start=time.monotonic();e.send('time 1');e.until('move ',1)
    assert time.monotonic()-start<.5

with Engine() as e:
    e.send('xboard');e.send('new');e.send('force')
    for fen,reason in (
        ('7k/8/8/8/8/8/8/K7 w - - 0 1','Insufficient material'),
        ('7k/8/8/8/8/8/8/RK6 w - - 100 51','50-move rule'),
        ('7k/5Q2/6K1/8/8/8/8/8 b - - 0 1','Stalemate'),
    ):
        e.send('force');e.send('setboard '+fen);e.send('go')
        assert reason in e.until('1/2-1/2 ',2)
    e.send('new');e.send('force')
    for move in ('g1f3','g8f6','f3g1','f6g8')*2:e.send('usermove '+move)
    e.send('go');assert 'Threefold repetition' in e.until('1/2-1/2 ',2)
    e.send('undo');e.send('undo');e.send('remove');e.send('sd 1')
    e.send('go');e.until('move ',2)

if os.name=='nt' and os.environ.get('PETIR_TEST_SKIP_OOM')!='1':
    # Force a real allocation failure without exhausting the host's memory.
    import ctypes
    from ctypes import wintypes as w
    class BasicLimit(ctypes.Structure):
        _fields_=[('ProcessTime',ctypes.c_int64),('JobTime',ctypes.c_int64),('Flags',w.DWORD),
                  ('MinWorkingSet',ctypes.c_size_t),('MaxWorkingSet',ctypes.c_size_t),
                  ('ActiveProcesses',w.DWORD),('Affinity',ctypes.c_size_t),('Priority',w.DWORD),('Scheduling',w.DWORD)]
    class IOCounters(ctypes.Structure):
        _fields_=[(name,ctypes.c_uint64) for name in ('ReadOps','WriteOps','OtherOps','ReadBytes','WriteBytes','OtherBytes')]
    class ExtendedLimit(ctypes.Structure):
        _fields_=[('Basic',BasicLimit),('IO',IOCounters),('ProcessMemory',ctypes.c_size_t),
                  ('JobMemory',ctypes.c_size_t),('PeakProcessMemory',ctypes.c_size_t),('PeakJobMemory',ctypes.c_size_t)]
    kernel=ctypes.WinDLL('kernel32',use_last_error=True)
    kernel.CreateJobObjectW.argtypes=[ctypes.c_void_p,w.LPCWSTR];kernel.CreateJobObjectW.restype=w.HANDLE
    kernel.SetInformationJobObject.argtypes=[w.HANDLE,ctypes.c_int,ctypes.c_void_p,w.DWORD];kernel.SetInformationJobObject.restype=w.BOOL
    kernel.AssignProcessToJobObject.argtypes=[w.HANDLE,w.HANDLE];kernel.AssignProcessToJobObject.restype=w.BOOL
    kernel.CloseHandle.argtypes=[w.HANDLE];kernel.CloseHandle.restype=w.BOOL
    job=kernel.CreateJobObjectW(None,None)
    assert job
    try:
        limits=ExtendedLimit();limits.Basic.Flags=0x100;limits.ProcessMemory=128*1024*1024
        assert kernel.SetInformationJobObject(job,9,ctypes.byref(limits),ctypes.sizeof(limits))
        with Engine() as e:
            e.send('uci');e.until('uciok')
            assert kernel.AssignProcessToJobObject(job,w.HANDLE(int(e.p._handle))),ctypes.get_last_error()
            e.send('setoption name Hash value 1024');e.until('info string ',allow_error=True)
            e.send('setoption name Clear Hash');e.send('setoption name OwnBook value false')
            e.send('position startpos');e.send('go depth 5');assert best(e)!='0000'
            e.send('setoption name Hash value 16');e.send('position startpos');e.send('go depth 1');assert best(e)!='0000'
    finally:kernel.CloseHandle(job)
elif os.name=='nt':
    print('Forced OOM recovery is tested in regular builds; MSVC AddressSanitizer aborts on OOM.')

print('UCI and WinBoard protocol, cancellation, draw, and hash-failure tests passed')
