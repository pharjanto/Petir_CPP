"""Measure exact-bound overwrites and benchmark protection; never plays matches."""
import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import hashlib
import json
from pathlib import Path
import sys
sys.path.insert(0, str(Path(__file__).resolve().parent))
from hash_counter_stats import Engine

def validate_store(s, protected):
    assert s['store_calls'] >= s['store_attempts'] == s['store_writes']+s['store_protected']
    assert s['store_same_key_equal_depth'] <= s['store_same_key'] <= s['store_attempts']
    assert s['store_protect_eligible'] <= s['store_exact_to_bound_equal'] <= s['store_same_key_equal_depth']
    for key in ('slots','bounds','ages','counters'):
        assert sum(s['store_downgrade_'+key]) == s['store_exact_to_bound_equal']
    assert s['store_protected'] == (s['store_protect_eligible'] if protected else 0)

def run(args, suite, mode, shard):
    engines={}
    rows=[]
    try:
        for label in ('baseline','candidate','baseline_stats','candidate_stats'):
            engines[label]=Engine(getattr(args,label),args.hash,2+2*shard)
        for index, position in enumerate(suite[shard::args.jobs]):
            reset=mode=='fresh'
            labels=['baseline','candidate'] if index%2==0 else ['candidate','baseline']
            result={label:engines[label].search(position,reset,False) for label in labels}
            for label in ('baseline','candidate'):
                diagnostic=engines[label+'_stats'].search(position,reset,True)
                if diagnostic['uci']!=result[label]['uci']:
                    raise AssertionError(f'{label} instrumentation changes search: {mode}/{position["name"]}')
                validate_store(diagnostic['stats'],label=='candidate')
                result[label]['stats']=diagnostic['stats']
            rows.append(dict(position,mode=mode,shard=shard+1,results=result))
    finally:
        for engine in engines.values():engine.close()
    (args.out/f'{mode}-{shard+1}.json').write_text(json.dumps(rows,indent=1)+'\n')
    print(f'{mode} shard {shard+1}/{args.jobs}: {len(rows)} baseline/candidate searches validated',flush=True)
    return rows

def pct(n,d):return f'{100*n/d:.3f}%' if d else 'n/a'
def aggregate(rows,label,field):return sum(r['results'][label]['stats'][field] for r in rows)

def report(rows,args):
    lines=['# Protect equal-depth exact hash entries — 2026-10-20','',
        'Branch `hash-preserve-exact`, base `a2bc9a04dedd62b688661cca756d1ecafb117b97` (published timeReserve + 15 ms overhead build). Main is unchanged.','',
        'The measurement baseline preserves the existing search and replacement rules. Instrumentation observes the actual destination of each store before mutation, in both bucket slots. A new store into an empty slot counts as a write; an eviction of a different key is not a same-position update. Bucket-demotion copies are excluded from write counts.','',
        'An overwrite event means: same full key, same stored depth (quarter plies), existing exact bound, new upper or lower bound. All ages and counter contexts count in the broad statistic. The candidate protects only events with the same halfmove counter and current generation, retaining the entire old entry, including its move. Older entries and changed counters update normally. No shallower-move updates or other hash experiments are included.','',
        f'{len(rows)//2} positions, depth {args.depth}, Hash {args.hash} MiB, fresh and carried tables, {args.jobs} workers pinned to separate physical cores (logical CPUs 2,4,...). Baseline and normal candidate alternate run order. Both are independently repeated by instrumented builds and required to match all ordinary UCI output after stripping timing fields. Only normal Release builds supply timings. One timed pass per mode; reported time is the sum of engine search times.','',
        '## Frequency on unchanged baseline','',
        '| Table | All writes | Same-key equal-depth writes | Exact→bound events | % of equal-depth same-key writes | % of all writes | Protectable events | Protectable % of equal-depth same-key writes |',
        '|---|---:|---:|---:|---:|---:|---:|---:|']
    for mode in ('fresh','carry'):
        subset=[r for r in rows if r['mode']==mode]
        writes=aggregate(subset,'baseline','store_writes');equal=aggregate(subset,'baseline','store_same_key_equal_depth')
        events=aggregate(subset,'baseline','store_exact_to_bound_equal');eligible=aggregate(subset,'baseline','store_protect_eligible')
        lines.append(f'| {mode} | {writes:,} | {equal:,} | {events:,} | {pct(events,equal)} | {pct(events,writes)} | {eligible:,} | {pct(eligible,equal)} |')
    lines += ['', '| Table | First / second slot | Upper / lower incoming bound | Current / previous / older generation | Same / different counter |','|---|---|---|---|---|']
    for mode in ('fresh','carry'):
        subset=[r for r in rows if r['mode']==mode]
        cells=[]
        for field in ('slots','bounds','ages','counters'):
            values=[r['results']['baseline']['stats']['store_downgrade_'+field] for r in subset]
            cells.append(' / '.join(f'{sum(v[i] for v in values):,}' for i in range(len(values[0]))))
        lines.append('| '+mode+' | '+' | '.join(cells)+' |')
    lines += ['', '## Fixed-depth benchmark','',
        '| Table | Baseline nodes | Protected nodes | Node change | Baseline seconds | Protected seconds | Time change | Candidate prevented writes |',
        '|---|---:|---:|---:|---:|---:|---:|---:|']
    for mode in ('fresh','carry'):
        subset=[r for r in rows if r['mode']==mode]
        bn=sum(r['results']['baseline']['nodes'] for r in subset);cn=sum(r['results']['candidate']['nodes'] for r in subset)
        bt=sum(r['results']['baseline']['engine_ms'] for r in subset);ct=sum(r['results']['candidate']['engine_ms'] for r in subset)
        lines.append(f'| {mode} | {bn:,} | {cn:,} | {100*(cn/bn-1):+.3f}% | {bt/1000:.3f} | {ct/1000:.3f} | {100*(ct/bt-1):+.3f}% | {aggregate(subset,"candidate","store_protected"):,} |')
    lines += ['', 'Baseline opportunities and candidate prevented writes come from different search trees; their totals need not match.','',
        '| Phase | Mode | Baseline exact→bound events | % of equal-depth same-key writes | Node change | Time change |',
        '|---|---|---:|---:|---:|---:|']
    for group in sorted({r['group'] for r in rows}):
        for mode in ('fresh','carry'):
            subset=[r for r in rows if r['group']==group and r['mode']==mode]
            events=aggregate(subset,'baseline','store_exact_to_bound_equal');equal=aggregate(subset,'baseline','store_same_key_equal_depth')
            changes=[100*(sum(r['results']['candidate'][f] for r in subset)/sum(r['results']['baseline'][f] for r in subset)-1) for f in ('nodes','engine_ms')]
            lines.append(f'| {group} | {mode} | {events:,} | {pct(events,equal)} | {changes[0]:+.3f}% | {changes[1]:+.3f}% |')
    lines += ['', '## Sample positions with the most overwrite events','',
        '| Position | Mode | Exact→bound events | Protectable | % of equal-depth same-key writes |','|---|---|---:|---:|---:|']
    for r in sorted(rows,key=lambda r:r['results']['baseline']['stats']['store_exact_to_bound_equal'],reverse=True)[:12]:
        s=r['results']['baseline']['stats']
        lines.append(f'| {r["name"]} | {r["mode"]} | {s["store_exact_to_bound_equal"]:,} | {s["store_protect_eligible"]:,} | {pct(s["store_exact_to_bound_equal"],s["store_same_key_equal_depth"])} |')
    changes=sum(r['results']['candidate']['bestmove']!=r['results']['baseline']['bestmove'] for r in rows)
    lines += ['',f'All {len(rows)} baseline and {len(rows)} candidate searches match their respective instrumented builds. Best moves changed in {changes}/{len(rows)} searches. Each raw record includes the position command/FEN and counters.','',
        'No strength match was started; these results do not establish Elo. Raw results and executable checksums: `build/hash-preserve-exact/results/results.json` and `BUILD_MANIFEST.json`.','']
    return '\n'.join(lines)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    for label in ('baseline','baseline-stats','candidate','candidate-stats'):
        parser.add_argument('--'+label,type=Path,required=True)
    parser.add_argument('--out',type=Path,required=True)
    parser.add_argument('--jobs',type=int,default=6)
    parser.add_argument('--depth',type=int,default=12)
    parser.add_argument('--hash',type=int,default=64)
    args=parser.parse_args()
    if not 1<=args.jobs<=10:parser.error('jobs must be 1..10, leaving two physical cores idle')
    args.out=args.out.resolve();args.out.mkdir(parents=True,exist_ok=True)
    metadata={'base_commit':'a2bc9a04dedd62b688661cca756d1ecafb117b97','depth':args.depth,'hash_mb':args.hash,'jobs':args.jobs,'builds':{}}
    for label in ('baseline','baseline_stats','candidate','candidate_stats'):
        exe=getattr(args,label).resolve();setattr(args,label,exe)
        metadata['builds'][label]={'path':str(exe),'sha256':hashlib.sha256(exe.read_bytes()).hexdigest()}
    print(json.dumps(metadata,indent=2),flush=True)
    suite=[{'name':name,'group':group,'position':position,'depth':args.depth}
           for name,group,position,_ in json.loads((Path(__file__).parent/'eval_diff/bench-games.json').read_text())]
    rows=[]
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures=[pool.submit(run,args,suite,mode,shard) for mode in ('fresh','carry') for shard in range(args.jobs)]
        for future in as_completed(futures):rows+=future.result()
    rows.sort(key=lambda r:(r['mode'],r['name']))
    (args.out/'results.json').write_text(json.dumps({'metadata':metadata,'results':rows},indent=1)+'\n')
    (args.out.parent/'BUILD_MANIFEST.json').write_text(json.dumps(metadata,indent=2)+'\n')
    text=report(rows,args)
    dest=Path(__file__).parents[1]/'reports/hash-preserve-exact-20261020.md';dest.parent.mkdir(exist_ok=True)
    dest.write_text(text,encoding='utf-8')
    print(text.encode('ascii','backslashreplace').decode('ascii'),flush=True)

if __name__=='__main__':main()
