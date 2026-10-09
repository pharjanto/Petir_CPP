"""End-to-end move deadlines, including late clocks and long +0 games."""
import contextlib, json, queue, re, subprocess, sys, threading, time

class Engine:
    def __init__(self, executable):
        self.p=subprocess.Popen([executable],stdin=subprocess.PIPE,stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,text=True,bufsize=1)
        self.lines=queue.Queue()
        def read():
            for line in self.p.stdout:self.lines.put((time.perf_counter(),line.rstrip()))
        threading.Thread(target=read,daemon=True).start()
        self.send('uci');self.until('uciok')
        self.send('setoption name OwnBook value false')
    def send(self,line):self.p.stdin.write(line+'\n');self.p.stdin.flush()
    def until(self,prefix,timeout=5):
        end=time.perf_counter()+timeout;seen=[]
        while time.perf_counter()<end:
            stamp,line=self.lines.get(timeout=max(.001,end-time.perf_counter()));seen.append(line)
            assert not line.startswith(('Error:','Illegal move:','info string ')),seen
            if line.startswith(prefix):return stamp,line,seen
        raise AssertionError((prefix,seen))
    def ready(self):self.send('isready');self.until('readyok')
    def close(self):
        if self.p.poll() is None:self.send('quit');self.p.wait(timeout=5)
        assert self.p.returncode==0,self.p.returncode

measurements=[]
executable=sys.argv[1]

def timed_reply(engine,command,prefix,remaining,label,allow_terminal=False):
    start=time.perf_counter();engine.send(command)
    stamp,line,seen=engine.until(prefix,max(1,remaining/1000+1))
    elapsed=(stamp-start)*1000
    assert elapsed<remaining,(label,remaining,elapsed,seen[-10:])
    move=line.split()[1]
    assert re.fullmatch(r'[a-h][1-8][a-h][1-8][qrbn]?',move) or (allow_terminal and move=='0000'),(label,line,seen[-10:])
    if move!='0000':
        if prefix=='bestmove ':
            infos=[row.split() for row in seen if row.startswith('info depth ')]
            assert infos and infos[-1][3:5]==['score','cp'],(label,seen[-10:])
            assert infos[-1][infos[-1].index('pv')+1]==move,(label,seen[-10:])
        else:
            infos=[row.split() for row in seen if re.match(r'^\d+ -?\d+ \d+ \d+ ',row)]
            assert infos and infos[-1][4]==move,(label,seen[-10:])
    measurements.append(dict(case=label,clock_ms=remaining,reply_ms=round(elapsed,3)))
    return move

with contextlib.ExitStack() as stack:
    e=Engine(executable);stack.callback(e.close)
    # Two genuine search workers provide concurrent CPU pressure while the GUI
    # reader threads drain their output. Timing assertions retain the real clock.
    for _ in range(2):
        load=Engine(executable);stack.callback(load.close)
        load.send('position startpos');load.send('go infinite')
    boards=(
        'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR',
        'r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R',
    )
    for hash_mb in (16,256):
        e.send(f'setoption name Hash value {hash_mb}');e.ready()
        for board in boards:
            for side in ('w','b'):
                fen=f'{board} {side} KQkq - 0 120'
                for remaining in (20,50,100,200,500,1000):
                    e.send('position fen '+fen);e.ready()
                    move=timed_reply(e,f'go wtime {remaining} btime {remaining}',
                        'bestmove ',remaining,f'UCI/{hash_mb}/{side}/{board}')
                    # The returned fallback/completed move must actually replay.
                    e.send('position fen '+fen+' moves '+move);e.ready()
    for remaining in (100,200,500):
        e.send('position fen '+boards[1]+' w KQkq - 0 1');e.ready()
        timed_reply(e,f'go movetime {remaining}','bestmove ',remaining,'fixed movetime')
    for side in ('w','b'):
        e.send(f'position fen {boards[1]} {side} KQkq - 0 120');e.ready()
        clocks='wtime 100 btime 30000' if side=='w' else 'wtime 30000 btime 100'
        timed_reply(e,f'go {clocks} winc 10000 binc 10000','bestmove ',100,'UCI increment/'+side)
    e.send('position startpos');e.ready()
    timed_reply(e,'go movetime 500 wtime 100 btime 100','bestmove ',100,'movetime capped by clock')

    e.send('xboard');e.send('protover 2')
    assert 'Move Overhead -spin 50' in e.until('feature ')[1]
    e.send('new');e.send('force');e.send('level 0 1 0')
    e.send('option Move Overhead=50')
    for side in ('w','b'):
        for remaining in (20,50,100,200,500,1000):
            e.send('force');e.send(f'setboard {boards[1]} {side} KQkq - 0 120')
            e.send(f'time {remaining//10}');e.send(f'otim {remaining//10}')
            e.send('ping 9');e.until('pong 9')
            timed_reply(e,'go','move ',remaining,'WinBoard/'+side)
    # Tighten a running search, rather than merely testing its initial budget.
    e.send('force');e.send('new');e.send('level 0 3 0');e.send('go')
    time.sleep(.02)
    timed_reply(e,'time 10','move ',100,'late WinBoard clock')
    e.send('force');e.send('level 0 1 10');e.send('setboard '+boards[1]+' w KQkq - 0 120')
    e.send('time 10');e.send('otim 3000');e.send('ping 10');e.until('pong 10')
    timed_reply(e,'go','move ',100,'WinBoard increment')

    # A long game must not repeatedly spend a 250 ms minimum at the end.
    # Both engines play successive legal moves on rook endings, with a 1 s bank
    # and no increment. Restart terminal boards without replenishing the bank.
    # The FEN counter deliberately starts beyond move 100.
    opponent=Engine(executable);stack.callback(opponent.close)
    e.send('uci');e.until('uciok');e.send('setoption name Hash value 64');e.ready()
    fen='6kr/5pp1/7p/8/8/P7/1PP5/RK6 w - - 0 120';moves=[];clock=[1000.0,1000.0]
    terminal_boards=0
    for ply in range(80):
        side=len(moves)%2;engine=e if side==0 else opponent
        engine.send('position fen '+fen+(' moves '+' '.join(moves) if moves else ''));engine.ready()
        remaining=max(1,int(clock[side]))
        before=len(measurements)
        move=timed_reply(engine,f'go wtime {int(clock[0])} btime {int(clock[1])}',
            'bestmove ',remaining,f'long +0 endgame/ply {ply+1}',allow_terminal=True)
        clock[side]-=measurements[before]['reply_ms']
        if move=='0000':
            engine.send('d');terminal_fen=engine.until('')[1]
            perft=subprocess.run([executable,'--perft','1',terminal_fen],
                capture_output=True,text=True,check=True,timeout=5)
            assert perft.stdout.startswith('0 nodes'),(terminal_fen,perft.stdout)
            terminal_boards+=1;moves=[]
        else:
            moves.append(move)
    assert min(clock)>0,clock
    assert terminal_boards<10,terminal_boards
    e.send('position fen '+fen+(' moves '+' '.join(moves) if moves else ''));e.ready()

print(json.dumps(dict(deadline_checks=len(measurements),
    maximum_reply_ms=max(row['reply_ms'] for row in measurements),
    ending_clock_ms=clock,terminal_boards=terminal_boards),indent=2))
if len(sys.argv)>2:
    with open(sys.argv[2],'w') as output:
        json.dump(measurements,output,indent=2)
