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
    # Every search explores root moves from depth 1, including with a retained
    # root entry. Child hash cutoffs still save work on the repeated position.
    # ucinewgame also clears the history and killer tables, so the node counts
    # do not depend on what the earlier searches in this test left behind.
    for mtdf,depth in ((False,7),(True,4)):
        e.send('setoption name MTDf value '+str(mtdf).lower())
        e.send('ucinewgame')
        e.send('setoption name Clear Hash')
        searches=[]
        for run in range(2):
            e.send('position startpos moves e2e4 e7e5 g1f3');e.transcript.clear()
            e.send(f'go depth {depth}');move=best(e,5);infos=search_infos(e.transcript)
            assert infos and int(infos[0].split()[2])==1,infos
            assert {int(line.split()[2]) for line in infos}==set(range(1,depth+1)),infos
            assert int(infos[-1].split()[2])==depth,infos
            assert_completed_best(e,move)
            nodes=int(next(line.split()[2] for line in e.transcript if line.startswith('info nodes ')))
            assert nodes>0,e.transcript
            searches.append((move,infos[-1].split()[5],nodes))
        # The second search starts from the evaluation correction the first one
        # learned, so its score may move a few points.
        assert abs(int(searches[0][1])-int(searches[1][1]))<=20 and searches[1][2]<searches[0][2],searches
        # A shallow request also searches the root despite the deeper entry.
        e.send('position startpos moves e2e4 e7e5 g1f3');e.transcript.clear()
        e.send('go depth 1');move=best(e,5);infos=search_infos(e.transcript)
        assert infos and {int(line.split()[2]) for line in infos}=={1},infos
        assert int(next(line.split()[2] for line in e.transcript if line.startswith('info nodes ')))>0,e.transcript
        assert_completed_best(e,move)
    # Follow the engine move and predicted reply from its previous PV, without
    # clearing the cached child entries that become the next root.
    for mtdf in (False,True):
        # Normal search supplies a full PV; MTD(f) can report only the root move.
        e.send('setoption name MTDf value false')
        e.send('setoption name Clear Hash')
        e.send('position startpos moves e2e4 e7e5 g1f3');e.transcript.clear()
        e.send('go depth 7');move=best(e,5)
        pvs=[line.split(' pv ',1)[1].split() for line in search_infos(e.transcript) if ' pv ' in line]
        pvs=[pv for pv in pvs if len(pv)>=2 and pv[0]==move]
        assert pvs,e.transcript
        continuation='e2e4 e7e5 g1f3 '+' '.join(pvs[-1][:2])
        e.send('setoption name MTDf value '+str(mtdf).lower())
        e.send('position startpos moves '+continuation);e.transcript.clear()
        e.send('go depth 7');move=best(e,5);infos=search_infos(e.transcript)
        assert infos and int(infos[0].split()[2])==1,infos
        assert {int(line.split()[2]) for line in infos}==set(range(1,8)),infos
        assert_completed_best(e,move)
    e.send('setoption name MTDf value false')
    for limit in ('movetime 0','wtime 0 btime 0','wtime 20 btime 20'):
        e.send('position startpos');start=time.monotonic();e.send('go '+limit)
        assert best(e,1)!='0000' and time.monotonic()-start<(.02 if limit=='wtime 20 btime 20' else .1),limit
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
    e.send('uci');e.until('uciok');e.send('setoption name OwnBook value false')
    # Warm search state must not leak into a subsequent emergency reply.
    e.send('position startpos');e.send('go depth 4');best(e)
    for side in ('w','b'):
        fen=f'7k/8/8/8/8/8/8/KQ6 {side} - - 0 1'
        evaluation=subprocess.run([sys.argv[1],'--eval',fen],capture_output=True,text=True,check=True,timeout=5)
        score=int(evaluation.stdout.split()[0])
        assert (score>500 if side=='w' else score<-500),(side,score)
        for overhead,limit in ((50,'movetime 0'),(50,'movetime 50'),(50,'wtime 20 btime 20'),
                               (0,'movetime 1'),(0,'movetime 2'),(0,'wtime 5 btime 5')):
            e.send(f'setoption name Move Overhead value {overhead}')
            e.send('position fen '+fen);e.transcript.clear();e.send('go '+limit)
            move=best(e,1);infos=search_infos(e.transcript)
            assert len(infos)==1 and infos[0].split()[2]=='0',e.transcript
            assert int(infos[0].split()[5])==score,e.transcript
            assert infos[0].split()[9]=='0',e.transcript
            assert infos[0].split(' pv ')[1].split()==[move],e.transcript
            # Replay the reported PV to check its legality for this root.
            e.send('position fen '+fen+' moves '+move);e.send('isready');e.until('readyok')
    # A forced reply can bypass iterative deepening even with ample time.
    e.send('position fen 7k/7R/6K1/8/8/8/8/8 b - - 0 1');e.transcript.clear()
    e.send('go depth 3');move=best(e,1)
    assert move=='h8g8',e.transcript
    assert_completed_best(e,move)
    assert search_infos(e.transcript)[-1].split()[2]=='0',e.transcript
    # Stopping before a worker starts still supplies final info for its move.
    for _ in range(3):
        e.send('position startpos');e.transcript.clear()
        e.send('go infinite');e.send('stop');assert_completed_best(e,best(e,2))
    e.send('xboard');e.send('force');e.send('level 0 1 0');e.send('option Move Overhead=50')
    for posting in ('post','nopost'):
        e.send('force');e.send('setboard '+fen);e.send('time 2');e.send('otim 2');e.send(posting)
        barrier(e);e.transcript.clear();e.send('go');move=e.until('move ',1).split()[1]
        infos=[line.split() for line in e.transcript if re.match(r'^\d+ -?\d+ \d+ \d+ ',line)]
        if posting=='post':
            assert len(infos)==1,e.transcript
            assert infos[0][:2]==['0',str(score)] and infos[0][3:]==['0',move],e.transcript
        else:
            assert not infos,e.transcript

with Engine() as e:
    e.send('uci');e.until('uciok');e.send('setoption name OwnBook value false')
    # The same rook position is winning with counter 0, but draws at counter 99
    # in this depth-3 search. A retained result must not override the new counter.
    for board,turn in (('7k/8/8/8/8/8/8/KR6','w'),('kr6/8/8/8/8/8/8/7K','b')):
        searches=[]
        for count,clear in ((0,True),(99,False),(99,True),(0,False)):
            if clear:e.send('setoption name Clear Hash')
            e.send(f'position fen {board} {turn} - - {count} 1');e.transcript.clear()
            e.send('go depth 3');move=best(e,5);assert_completed_best(e,move)
            score=int(search_infos(e.transcript)[-1].split()[5])
            nodes=int(next(line.split()[2] for line in e.transcript if line.startswith('info nodes ')))
            searches.append((score,nodes))
        assert searches[0][0]>400 and searches[3][0]>400,searches
        assert searches[1][0]==searches[2][0]==0,searches
        assert searches[1][1]>0 and searches[3][1]>0,searches

with Engine() as e:
    e.send('uci');e.until('uciok')
    # Restricted roots bypass OwnBook and retained root moves, and preserve a
    # matching final PV even when there is only one permitted move.
    for mtdf in (False,True):
        e.send('setoption name MTDf value '+str(mtdf).lower())
        e.send('position startpos');e.send('go depth 3');best(e)
        for command,allowed in (
            ('go searchmoves a2a3 depth 3',{'a2a3'}),
            ('go depth 3 searchmoves a2a3 h2h3 a2a3',{'a2a3','h2h3'}),
            ('go searchmoves h2h3 movetime 0',{'h2h3'}),
            ('go searchmoves a2a3 nodes 0',{'a2a3'}),
        ):
            e.send('position startpos');e.transcript.clear();e.send(command)
            move=best(e,3);assert move in allowed,(command,e.transcript)
            assert_completed_best(e,move)
            assert all(line.split(' pv ')[1].split()[0] in allowed for line in search_infos(e.transcript)),e.transcript
        for fen,move in (
            ('r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1','e1g1'),
            ('7k/8/8/3pP3/8/8/8/K7 w - d6 0 1','e5d6'),
            ('7k/7R/6K1/8/8/8/8/8 b - - 0 1','h8g8'),
            ('7k/8/8/8/8/8/4r3/4K3 w - - 0 1','e1d1'),
            ('7k/P7/8/8/8/8/8/7K w - - 0 1','a7a8q'),
            ('7k/P7/8/8/8/8/8/7K w - - 0 1','a7a8n'),
            ('7k/8/8/8/8/8/p7/7K b - - 0 1','a2a1n'),
        ):
            e.send('position fen '+fen);e.transcript.clear()
            e.send('go searchmoves '+move+' depth 3');assert best(e,3)==move,e.transcript
            assert_completed_best(e,move)
            assert int(search_infos(e.transcript)[-1].split()[2])==3,e.transcript
        # Node-only requests finish without stop, including budgets smaller
        # than one iteration. Capture-rich roots also exercise quiescence.
        for position in ('startpos','fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1'):
            for count in (0,1,2,17,50,500,2000,10000):
                e.send('position '+position);e.transcript.clear();e.send(f'go nodes {count}')
                move=best(e,3);assert move!='0000';assert_completed_best(e,move)
                for line in search_infos(e.transcript):
                    assert 0<=int(line.split()[9])<=count,(count,e.transcript)
                for line in e.transcript:
                    if line.startswith('info nodes '):assert int(line.split()[2])<=count,(count,e.transcript)
        e.send('position startpos');e.transcript.clear();e.send('go nodes 4294967296 depth 1')
        assert best(e,3)!='0000';assert search_infos(e.transcript)[-1].split()[2]=='1',e.transcript
        # Limits reset for subsequent searches, including after nodes 0.
        e.send('go nodes 0');best(e)
        e.send('setoption name OwnBook value false');e.transcript.clear();e.send('go depth 3')
        assert best(e,3)!='0000';assert search_infos(e.transcript)[-1].split()[2]=='3',e.transcript
        for position in ('startpos','fen 7k/6Q1/6K1/8/8/8/8/8 b - - 0 1'):
            e.send('position '+position);e.send('go searchmoves depth 2');assert best(e,3)=='0000'
    # Explicit infinite/ponder requests hold results even after their node
    # budget is exhausted; the GUI remains responsive while they wait.
    for mode,release in (('infinite','stop'),('ponder','ponderhit')):
        e.send('position startpos');e.transcript.clear();e.send(f'go {mode} nodes 50 searchmoves a2a3')
        time.sleep(.05);e.send('isready');e.until('readyok')
        assert not any(line.startswith('bestmove ') for line in e.transcript),e.transcript
        e.send(release);assert best(e,3)=='a2a3'
    # Reject unsupported promotions/illegal moves without changing the board.
    for move in ('a7a8r','a7a8b','a7a6'):
        e.send('position fen 7k/P7/8/8/8/8/8/7K w - - 0 1')
        e.send('go depth 2 searchmoves '+move);e.until('info string ',allow_error=True)
        e.send('go depth 2 searchmoves a7a8q');assert best(e,3)=='a7a8q'
    # Mate uses full moves, reports UCI mate scores, and obeys other limits.
    for mtdf in (False,True):
        e.send('setoption name MTDf value '+str(mtdf).lower())
        for fen,move in (('7k/8/5KQ1/8/8/8/8/8 w - - 0 1','g6g7'),
                         ('8/8/8/8/8/5kq1/8/7K b - - 0 1','g3g2')):
            e.send('setoption name Clear Hash');e.send('position fen '+fen);e.transcript.clear()
            e.send('go mate 1 searchmoves '+move);assert best(e,3)==move,e.transcript
            assert search_infos(e.transcript)[-1].split()[3:6]==['score','mate','1'],e.transcript
        for fen in ('k7/8/8/1QK5/8/8/8/8 w - - 0 1','8/8/8/8/1qk5/8/8/K7 b - - 0 1'):
            e.send('setoption name Clear Hash');e.send('position fen '+fen);e.transcript.clear()
            e.send('go mate 2');assert best(e,3)!='0000'
            assert search_infos(e.transcript)[-1].split()[3:6]==['score','mate','2'],e.transcript
            assert int(search_infos(e.transcript)[-1].split()[2])<=4,e.transcript
        e.send('setoption name Clear Hash')
        e.send('position fen 7k/8/5K2/6Q1/8/8/8/8 b - - 0 1');e.transcript.clear()
        e.send('go mate 1');assert best(e,3)=='h8h7'
        assert search_infos(e.transcript)[-1].split()[3:6]==['score','mate','-1'],e.transcript
        for command,depth in (('go mate 1',2),('go mate 3 depth 1',1),('go depth 1 mate 3',1)):
            e.send('position startpos');e.transcript.clear();e.send(command)
            assert best(e,3)!='0000';assert int(search_infos(e.transcript)[-1].split()[2])==depth,e.transcript
        e.send('position startpos');e.transcript.clear();e.send('go mate 10 nodes 50')
        assert best(e,3)!='0000'
        assert all(int(line.split()[9])<=50 for line in search_infos(e.transcript)),e.transcript
    e.send('position startpos');e.transcript.clear();e.send('go nodes 1000000000 movetime 0 searchmoves a2a3')
    assert best(e,1)=='a2a3';assert search_infos(e.transcript)[-1].split()[2]=='0',e.transcript

with Engine() as e:
    e.send('uci');e.until('uciok')
    assert 'option name UCI_AnalyseMode type check default false' in e.transcript
    e.send('setoption name Hash value 16')
    # Compare every completed iteration, excluding wall-clock time. Repeated
    # analysis must match even after another search has populated the tables.
    for mtdf in (False,True):
        e.send('setoption name MTDf value '+str(mtdf).lower())
        for analysis_command in ('option','infinite'):
            e.send('setoption name UCI_AnalyseMode value '+str(analysis_command=='option').lower())
            searches=[]
            for run in range(2):
                e.send('position startpos');e.transcript.clear()
                e.send('go depth 6'+(' infinite' if analysis_command=='infinite' else ''))
                if analysis_command=='infinite':
                    e.until('info depth 6 ')
                    # A root update can precede iteration completion. Compare
                    # the deterministic prefix before the asynchronous stop.
                    analysis_infos=search_infos(e.transcript)
                    e.send('stop')
                move=best(e,5)
                if analysis_command=='option':analysis_infos=search_infos(e.transcript)
                infos=[re.sub(r' time \d+','',line) for line in analysis_infos]
                assert {int(line.split()[2]) for line in infos}==set(range(1,7)),infos
                searches.append((move,infos))
            assert searches[0]==searches[1],searches
    # Turning analysis off restores retained search state during ordinary play.
    e.send('setoption name OwnBook value false');e.send('setoption name MTDf value false')
    e.send('setoption name UCI_AnalyseMode value false')
    e.send('ucinewgame')
    nodes=[]
    for _ in range(2):
        e.send('position startpos');e.transcript.clear();e.send('go depth 6');best(e,5)
        nodes.append(int(search_infos(e.transcript)[-1].split()[9]))
    assert nodes[1]<nodes[0],nodes

with Engine() as e:
    e.send('xboard');e.send('protover 2');assert 'analyze=1' in e.until('feature ')
    e.send('new');e.send('force')
    start_fen='rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'
    def analysis_iterations(command):
        e.transcript.clear();e.send(command);e.until('6 ',5)
        infos=[tuple(line.split()[:2]+line.split()[3:]) for line in e.transcript
               if re.match(r'^[1-6] -?\d+ \d+ \d+ ',line)]
        e.send('ping 792');e.until('pong ')
        assert not any(line.startswith(('move ','bestmove ')) for line in e.transcript),e.transcript
        return infos
    first=analysis_iterations('analyze')
    e.send('exit');barrier(e)
    assert analysis_iterations('analyze')==first
    # Changing the board while analyzing starts a fresh search of that board.
    assert analysis_iterations('setboard '+start_fen)==first
    after_move=analysis_iterations('usermove e2e4')
    assert analysis_iterations('undo')==first
    assert analysis_iterations('usermove e2e4')==after_move
    e.send('exit');barrier(e)

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
        assert infos and int(infos[0][0])==1,infos
        assert {int(line[0]) for line in infos}==set(range(1,6)),infos
        assert int(infos[-1][0])==5,infos
        searches.append((move,infos[-1][1],int(infos[-1][3])))
        e.send('force');e.send('undo');barrier(e)
    # Re-searching can select another move tied at the same score.
    assert searches[0][1]==searches[1][1] and searches[1][2]<searches[0][2],searches

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
    start=time.monotonic();e.send('time 10');e.until('move ',1)
    assert time.monotonic()-start<.1

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
