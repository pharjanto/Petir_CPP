#include "engine.hpp"
#include "kpk.hpp"
#include "hash_stats.hpp"
#include <algorithm>
#include <sstream>
namespace petir {
int selfTest() {
    int failures=0,checks=0;
    auto check=[&](bool passed,const std::string& message) {++checks;if(!passed) {++failures;std::cerr<<"FAIL: "<<message<<'\n';}};
    using namespace petir_header;
    ClockLimits defaultLimits;defaultLimits.timed=true;defaultLimits.remainingMs=10000;
    auto defaultBudget=TimeManager::allocate(defaultLimits);
    check(defaultLimits.overheadMs==15 && defaultBudget.softMs==317 && defaultBudget.hardMs==951,
        "default 15 ms overhead retains the 30-move sudden-death horizon");
    defaultLimits.incrementMs=100;defaultBudget=TimeManager::allocate(defaultLimits);
    check(defaultBudget.softMs==397 && defaultBudget.hardMs==1191,
        "default overhead keeps the 80 percent increment contribution");
    defaultLimits.moveTimeMs=80;defaultBudget=TimeManager::allocate(defaultLimits);
    check(defaultBudget.softMs==65 && defaultBudget.hardMs==65,
        "fixed movetime reserves the default 15 ms overhead");
    ClockLimits tmLimits;tmLimits.timed=true;tmLimits.remainingMs=1000;tmLimits.overheadMs=50;
    auto tmBudget=TimeManager::allocate(tmLimits);
    check(tmBudget.softMs>0 && tmBudget.softMs<250 && tmBudget.hardMs<=950,"low-clock allocation has no 250 ms floor and reserves overhead");
    tmLimits.incrementMs=2000;const auto withIncrement=TimeManager::allocate(tmLimits);
    check(withIncrement.softMs>tmBudget.softMs && withIncrement.hardMs<=950,"increment improves the target without spending future increment before the move");
    tmLimits.remainingMs=50;
    check(TimeManager::allocate(tmLimits).emergency && TimeManager::allocate(tmLimits).hardMs==0,"clock below overhead selects the emergency path even with increment");
    tmLimits.remainingMs=60000;tmLimits.incrementMs=0;tmLimits.movesToGo=1;
    check(TimeManager::allocate(tmLimits).hardMs<=59950,"last move of a control still retains move overhead");
    check(TimeManager::allocate(tmLimits).softMs>=59800,"last move of a control may use the whole clock");
    tmLimits.movesToGo=3;tmBudget=TimeManager::allocate(tmLimits);
    check(tmBudget.softMs<=12000 && tmBudget.hardMs<=28000,"moves before a control keep a reserve and cannot spend the whole clock");
    tmLimits.movesToGo=0;tmBudget=TimeManager::allocate(tmLimits);
    check(tmBudget.hardMs==tmBudget.softMs*3,"sudden death keeps the 3x soft overrun");
    tmLimits.movesToGo=1;
    tmLimits.moveTimeMs=80;tmBudget=TimeManager::allocate(tmLimits);
    check(tmBudget.softMs==30 && tmBudget.hardMs==30,"fixed movetime includes response overhead");
    TimeManager timeManager;const auto received=TimeManager::nowMs();
    timeManager.start({true,false,10,20},received-100);
    check(timeManager.expired(true),"the hard deadline includes time spent preparing the search");
    timeManager.adjustSoft(100);
    check(timeManager.expired(true) && !timeManager.canStartIteration(0),"soft extensions cannot revive an expired hard deadline");
    timeManager.tighten(received,60000,0);
    check(timeManager.expired(true),"clock updates cannot extend the original hard deadline");
    timeManager.start({true,false,10000,20000},received);
    check(!timeManager.canStartIteration(15000),"iteration prediction rejects work that would exceed the hard budget");
    timeManager.start({true,false,100,1000},received);
    timeManager.tighten(received,0,50);
    check(timeManager.expired(true),"a late zero-clock update expires the deadline immediately");
    timeManager.start({},received);
    check(!timeManager.expired(true) && timeManager.canStartIteration(1000),"depth-only and infinite searches do not acquire a clock limit");
    // Compare the native scan with the legacy 16-bit lookup for every word
    // value in each block, including higher/lower occupied blocks respectively.
    std::uint64_t scanBits=0;
    check(petir_bitboard_mask::firstBit(pas::address(scanBits))==0,"empty bit scan retains legacy zero result");
    check(petir_bitboard_mask::lastBit(pas::address(scanBits))==-1,"empty reverse bit scan retains legacy minus-one result");
    bool scanMatches=true,reverseScanMatches=true;
    for(int block=0;block<4;++block) {
        for(std::uint32_t word=1;word<=65535;++word) {
            scanBits=std::uint64_t(word)<<(block*16);
            if(block<3)scanBits|=UINT64_C(0xffff000000000000);
            if(petir_bitboard_mask::firstBit(pas::address(scanBits))!=block*16+petir_bitboard_mask::firstBitTable[word])scanMatches=false;
            scanBits=(std::uint64_t(word)<<(block*16))|((UINT64_C(1)<<(block*16))-1);
            if(petir_bitboard_mask::lastBit(pas::address(scanBits))!=block*16+petir_bitboard_mask::firstBitTable[word])reverseScanMatches=false;
        }
    }
    check(scanMatches,"native bit scan matches legacy lookup across all 16-bit blocks");
    check(reverseScanMatches,"native reverse scan preserves the highest-word/least-bit order for every word");
    // Compare both count APIs with the old per-bit loop, including dense and
    // sparse boards whose bits span all four 16-bit words.
    bool countsMatch=true,singleBitMatches=true;
    auto checkCounts=[&](std::uint64_t bits) {
        int expected=0;
        for(auto remaining=bits;remaining;remaining&=remaining-1)++expected;
        if(petir_ueval::popCount(bits)!=expected
            || petir_ueval::popCountWords(pas::address(bits))!=expected)countsMatch=false;
        if(petir_ueval::hasSingleBit(bits)!=(expected==1))singleBitMatches=false;
    };
    checkCounts(0);checkCounts(UINT64_MAX);
    for(int bit=0;bit<64;++bit)checkCounts(UINT64_C(1)<<bit);
    for(int block=0;block<4;++block)for(std::uint64_t word=0;word<65536;++word) {
        checkCounts(word<<(block*16));
        checkCounts((word<<(block*16))|(UINT64_C(0xa5c39e714b286df0)&~(UINT64_C(0xffff)<<(block*16))));
    }
    check(countsMatch,"native bit counts match the legacy loop across all word values and mixed boards");
    check(singleBitMatches,"single-bit predicate agrees with an exact count, including zero and bit 63");
    alignas(std::uint64_t) std::uint16_t unalignedWords[5]{};
    const auto unalignedBits=UINT64_C(0xfedcba9876543210);
    std::memcpy(unalignedWords+1,&unalignedBits,sizeof(unalignedBits));
    check(petir_ueval::popCountWords(unalignedWords+1)==32,"pointer popcount supports a board without 64-bit alignment");
    Position start{};petir_udata::setInitialPosition(start);
    check(start.hashKey==3423812783342919194ULL,"Delphi starting hash and Random evaluation order");
    check(isConsistent(start),"starting board, rotated occupancy, material, and hash agree");
    // Evaluation regression scores. The mixed pawn-file fixtures include the
    // passed-pawn refinement and opposing rook activity, mirrored for Black
    // after the existing material draw scaling.
    // Include promoted black pieces in different 16-bit blocks.
    struct EvalOptimizationFixture {const char* name;const char* fen;int full[4];int narrow[4];};
    for(const auto& fixture:{
        EvalOptimizationFixture{"multiple rooks",
            "6k1/2r2rr1/1b4n1/3p4/r2N1b2/2P1B3/1R3P2/3Q1RK1 b - - 0 1",
            {-117,-75,75,117},{-117,-75,75,117}},
        EvalOptimizationFixture{"multiple knights",
            "6k1/2n2n2/1b3b2/3p4/n2N1b2/2P1B3/1R3P2/3Q1RK1 b - - 0 1",
            {742,840,-840,-742},{804,784,-784,-804}},
        EvalOptimizationFixture{"multiple bishops",
            "6k1/2b5/1b3b2/3p4/b2N1b2/2P1B3/1R3P2/3Q1RK1 b - - 0 1",
            {1073,1109,-1109,-1073},{1107,1087,-1087,-1107}},
        EvalOptimizationFixture{"multiple queens",
            "6k1/2q5/1q3q2/3p4/q2N1b2/2P1B3/1R3P2/3Q1RK1 b - - 0 1",
            {-1817,-1757,1757,1817},{-1664,-1684,1684,1664}},
        EvalOptimizationFixture{"white pawn files",
            "7k/8/8/P1PP1P1P/P2P1P2/8/8/KB5r w - - 0 1",
            {802,802,-802,-802},{802,802,-802,-802}},
        EvalOptimizationFixture{"black pawn files",
            "kb5R/8/8/p2p1p2/p1pp1p1p/8/8/7K b - - 0 1",
            {-802,-802,802,802},{-802,-802,802,802}}}) {
        int side=1;const auto position=fromFen(fixture.fen,side);const auto saved=position;
        const auto tag=std::string("evaluation optimization: ")+fixture.name;
        check(isConsistent(position),tag+" fixture is consistent");
        for(int sideToMove:{1,2})for(bool opponentPerspective:{false,true}) {
            const int index=(sideToMove-1)*2+int(opponentPerspective);
            const auto detail=tag+" side="+std::to_string(sideToMove)+" infhr="+std::to_string(opponentPerspective);
            const int actual=petir_ueval::evaluate(position,sideToMove,-20500,20500,opponentPerspective);
            check(actual==fixture.full[index],detail+" full score="+std::to_string(actual));
            check(petir_ueval::evaluate(position,sideToMove,0,1,opponentPerspective)==fixture.narrow[index],detail+" lazy score unchanged");
        }
        check(std::memcmp(&position,&saved,sizeof(position))==0,tag+" leaves the input unchanged");
    }
    check(sizeof(petir_hashing_header::thashrecord)==16,"halfmove-aware hash entries retain the 16-byte layout");
    // Same-position exact bounds are protected only in the same generation
    // and counter context. Exercise both destination slots and both colors.
    for(int color:{1,2})for(int slot:{0,1})for(int stored:{exactBound,upperBound,lowerBound})
        for(int incoming:{exactBound,upperBound,lowerBound})for(int old:{0,1})for(int differentCounter:{0,1}) {
        petir_hashing_header::clearHashTable();
        const auto key=color==2?~start.hashKey:start.hashKey;
        auto* entries=petir_hashing_header::bucket(key);
        const auto flags=static_cast<std::uint8_t>(petir_hashing_header::usedFlag
            | ((petir_hashing_header::generation-old)&petir_hashing_header::generationMask));
        if(slot==1) {
            entries[0].key=key^static_cast<std::uint64_t>(hashTableSize);
            entries[0].depth=200;entries[0].flags=petir_hashing_header::usedFlag|petir_hashing_header::generation;
        }
        auto& entry=entries[slot];
        entry.key=key;entry.depth=16;entry.boundType=stored;entry.flags=flags;
        entry.score=17;entry.move=123;entry.fiftyMoveCounter=0;
        const auto before=entry;
        petir_hashing::storeHash(start.hashKey,color,32,321,33,incoming,static_cast<std::uint8_t>(differentCounter));
        bool protect=false;
#ifdef PETIR_PRESERVE_EXACT
        protect=stored==exactBound && incoming!=exactBound && !old && !differentCounter;
#endif
        check(protect ? std::memcmp(&entry,&before,sizeof(entry))==0
            : entry.score==33 && entry.move==321 && entry.boundType==incoming && entry.fiftyMoveCounter==differentCounter,
            "equal-depth update respects exactness, age and counter in both slots and colors");
    }
    petir_hashing_header::clearHashTable();
    // Scores depend on the rule-50 counter even when the board key is identical.
    for(int color=1;color<=2;++color)for(int type:{exactBound,upperBound,lowerBound})for(int counter:{0,79,99,255}) {
        const int cachedMove=parseMove(color==1?"e2e4":"e7e5",start,color);
        const int expectedScore=type==upperBound?-101:type==lowerBound?101:17;
        const auto count=static_cast<std::uint8_t>(counter);
        petir_hashing::storeHash(start.hashKey,color,48,cachedMove,expectedScore,type,count);
        int alpha=-100,beta=100,score=777,move=0;bool avoidNull=true;
        const bool hit=petir_hashing::probeHash(start.hashKey,alpha,beta,8,color,score,move,avoidNull,count);
        const std::string tag=std::to_string(color)+"/"+std::to_string(type)+"/"+std::to_string(counter);
        check(hit && score==expectedScore && move==cachedMove,"reuse hash score with matching halfmove counter "+tag);
        score=777;move=0;avoidNull=true;
        const auto otherCount=static_cast<std::uint8_t>(counter==255?0:counter+1);
        const bool otherHit=petir_hashing::probeHash(start.hashKey,alpha,beta,8,color,score,move,avoidNull,otherCount);
        const bool relaxed=counter==0;
        check(otherHit==relaxed && score==(relaxed?expectedScore:777) && move==cachedMove && !avoidNull,"counter compatibility respects the fade margin "+tag);
    }
    for(int color=1;color<=2;++color) {
        const int cachedMove=parseMove(color==1?"e2e4":"e7e5",start,color);
        petir_hashing::storeHash(start.hashKey,color,48,cachedMove,200,upperBound,79);
        int alpha=-100,beta=100,score=777,move=0;bool avoidNull=false;
        (void)petir_hashing::probeHash(start.hashKey,alpha,beta,8,color,score,move,avoidNull,79);
        check(avoidNull,"matching halfmove counter permits cached null-move restriction");
        (void)petir_hashing::probeHash(start.hashKey,alpha,beta,8,color,score,move,avoidNull,80);
        check(!avoidNull,"different halfmove counter rejects cached null-move restriction");
        auto old=start;old.fiftyMoveCounter=79;
        petir_hashing::storeHash(start.hashKey,color,48,cachedMove,17,exactBound,79);
        check(petir_pv::getPv(old,color,1).text.starts_with(color==1?"e2e4":"e7e5"),"PV extraction reads matching halfmove counter");
        old.fiftyMoveCounter=80;
        check(petir_pv::getPv(old,color,1).text.empty(),"PV extraction rejects a different halfmove counter");
    }
    auto sameBoard=start;sameBoard.fiftyMoveCounter=99;
    check(petir_hashing_header::computeHashKey(sameBoard,1)==start.hashKey,"halfmove counter does not alter book/repetition hash keys");
    // Boundary uses rounded-up STORED depth, including deeper cached results.
    for(int color:{1,2})for(int type:{exactBound,upperBound,lowerBound})
    for(int depth:{8,10,48,504})for(bool reverse:{false,true}) {
        const int plies=((depth/2)+3)/4;
        for(int high:{79-16-plies,80-16-plies}) {
            if(high<1) continue;
            petir_hashing_header::clearHashTable();
            const int stored=reverse?high:high-1, current=reverse?high-1:high;
            const int value=type==upperBound?-101:type==lowerBound?101:17;
            petir_hashing::storeHash(start.hashKey,color,depth,123,value,type,static_cast<std::uint8_t>(stored));
            int alpha=-100,beta=100,score=777,move=0;bool avoidNull=true;
            const bool hit=petir_hashing::probeHash(start.hashKey,alpha,beta,8,color,score,move,avoidNull,static_cast<std::uint8_t>(current));
            const bool expected=high+plies+16<80;
            check(hit==expected && score==(expected?value:777) && move==123 && !avoidNull,"relaxed counter boundary uses rounded stored depth in both directions");
        }
    }
    for(int color:{1,2})for(int value:{-20000,-19900,-19899,19899,19900,20000}) {
        petir_hashing_header::clearHashTable();
        petir_hashing::storeHash(start.hashKey,color,48,123,value,exactBound,0);
        int alpha=-20500,beta=20500,score=777,move=0;bool avoidNull=false;
        const bool hit=petir_hashing::probeHash(start.hashKey,alpha,beta,8,color,score,move,avoidNull,1);
        check(hit==(value>-19900 && value<19900),"mate-range scores require exact counters");
        check(petir_hashing::probeHash(start.hashKey,alpha,beta,8,color,score,move,avoidNull,0) && score==value,"matching counters retain mate-range scores");
    }
    for(int color:{1,2}) {
        petir_hashing_header::clearHashTable();
        petir_hashing::storeHash(start.hashKey,color,48,123,200,upperBound,0);
        int alpha=-100,beta=100,score=777,move=0;bool avoidNull=true;
        check(!petir_hashing::probeHash(start.hashKey,alpha,beta,8,color,score,move,avoidNull,1) && !avoidNull,"relaxed low counters do not enable avoidNullMove");
        check(!petir_hashing::probeHash(start.hashKey,alpha,beta,8,color,score,move,avoidNull,0) && avoidNull,"exact low counters retain avoidNullMove");
    }
#ifdef PETIR_HASH_STATS
    // Exercise the classifier against real probes: bounds and depth can reject
    // an entry independently of the counter, and PV hits never save a cutoff.
    for(int color:{1,2})for(int type:{exactBound,upperBound,lowerBound}) {
        petir_hashing_header::clearHashTable();
        const int cachedMove=parseMove(color==1?"e2e4":"e7e5",start,color);
        const int value=type==upperBound?-101:type==lowerBound?101:17;
        petir_hashing::storeHash(start.hashKey,color,48,cachedMove,value,type,0);
        // Push the fixture into the second bucket slot as well.
        const auto otherKey=start.hashKey^static_cast<std::uint64_t>(hashTableSize);
        petir_hashing::storeHash(otherKey,color,64,cachedMove,value,type,0);
        auto countedProbe=[&](std::uint64_t key,int depth,int count,int nodeType,int alpha=-100,int beta=100) {
            petir_hash_stats::reset();
            const auto encodedKey=color==2?~key:key;
            auto* bucket=petir_hashing_header::bucket(encodedKey);
            const std::array<petir_hashing_header::thashrecord,2> saved={bucket[0],bucket[1]};
            petir_hash_stats::Probe diagnostic;
            int score=777,move=0;bool avoidNull=false;
            const bool hit=petir_hashing::probeHash(key,alpha,beta,depth,color,score,move,avoidNull,static_cast<std::uint8_t>(count),&diagnostic);
            petir_hash_stats::record(diagnostic,depth,alpha,beta,static_cast<std::uint8_t>(count),nodeType,hit);
            check(std::memcmp(saved.data(),bucket,sizeof(saved))==0,"hash diagnostics preserve both bucket entries");
            check(petir_hash_stats::counters.decisionMismatches==0,"hash classifier agrees with actual probe");
            return hit;
        };
        check(countedProbe(start.hashKey,8,0,petir_search::cutNode)
            && petir_hash_stats::counters.acceptedCutoffs==1,"hash diagnostics count an accepted second-slot cutoff");
        check(countedProbe(start.hashKey,8,2,petir_search::cutNode)
            && petir_hash_stats::counters.relaxedCutoffs==1
            && petir_hash_stats::counters.counterBlockedCutoffs==0,"hash diagnostics count a relaxed second-slot cutoff");
        check(!countedProbe(start.hashKey,8,62,petir_search::allNode)
            && petir_hash_stats::counters.counterBlockedCutoffs==1
            && petir_hash_stats::counters.blockedBandsByDifference[0][2]==1
            && petir_hash_stats::counters.blockedBounds[type]==1,"hash diagnostics count a cutoff blocked only by counter");
        check(!countedProbe(start.hashKey,64,2,petir_search::cutNode)
            && petir_hash_stats::counters.counterBlockedCutoffs==0
            && petir_hash_stats::counters.depthRejectedMismatches==1,"shallow mismatched entries are not lost cutoffs");
        const bool wideHit=countedProbe(start.hashKey,8,62,petir_search::cutNode,-200,200);
        check(!wideHit && (type==exactBound?petir_hash_stats::counters.counterBlockedCutoffs==1
            :petir_hash_stats::counters.boundRejectedMismatches==1),"noncutting bounds are not lost cutoffs");
        check(countedProbe(start.hashKey,8,0,petir_search::pvNode)
            && petir_hash_stats::counters.probes==0
            && petir_hash_stats::counters.pvProbesExcluded==1,"PV probes are excluded from lost cutoff counts");
        check(!countedProbe(start.hashKey,8,80,petir_search::cutNode)
            && petir_hash_stats::counters.blockedBands[1]==1,"hash diagnostics distinguish evaluation fade counters");
        check(!countedProbe(start.hashKey,8,100,petir_search::cutNode)
            && petir_hash_stats::counters.blockedBands[2]==1,"hash diagnostics distinguish draw-limit counters");
        check(!countedProbe(start.hashKey^UINT64_C(0x10000000000),8,0,petir_search::cutNode)
            && petir_hash_stats::counters.keyHits==0,"key misses cannot count as counter rejections");
    }
    petir_hash_stats::reset();
#endif
    petir_hashing_header::clearHashTable();
    std::uint64_t counts[]={1,20,400,8902,197281,4865609};
    for(int depth=0;depth<=5;++depth)check(perft(start,1,depth)==counts[depth],"start position perft "+std::to_string(depth));
    int sideToMove=1;auto kiwipete=fromFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",sideToMove);
    std::uint64_t kiwipeteCounts[]={1,48,2039,97862};
    for(int depth=1;depth<=3;++depth)check(perft(kiwipete,sideToMove,depth)==kiwipeteCounts[depth],"Kiwipete perft "+std::to_string(depth));
    auto ending=fromFen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",sideToMove);
    check(perft(ending,sideToMove,4)==43238,"rook ending / en passant perft 4");
    for(const char* fen:{
        "kn6/pp1Q4/1Q4Q1/4Q3/Q5Q1/2Q5/1Q3Q2/K2Q4 w - - 0 1",
        "k2q4/1q3q2/2q5/q5q1/4q3/1q4q1/PP1q4/KN6 b - - 0 1"}) {
        auto promoted=fromFen(fen,sideToMove);
        auto promotedMoves=legalMoves(promoted,sideToMove);
        check(promotedMoves.size()>100,"promoted queens generate beyond the legacy move-list capacity");
        check(isConsistent(promoted),"large move generation preserves board state");
        const int promotedEval=petir_ueval::evaluate(promoted,sideToMove,-20500,20500);
        check(promotedEval>8000 && promotedEval<20000,"promoted material evaluates within bounded scale tables");
    }
    tmovelist guardedMoves{};
    bool overflowRejected=false;
    try {(void)guardedMoves[257];}catch(const std::out_of_range&) {overflowRejected=true;}
    check(overflowRejected,"move-list overflow rejected even without general bounds checks");
    // Search buffers need no initial clear only if generation fully defines
    // every counted move and score. Compare outputs from two poison patterns.
    bool generationDefinesEntries=true;
    for(const char* fen:{
        startFen,
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "kn6/pp1Q4/1Q4Q1/4Q3/Q5Q1/2Q5/1Q3Q2/K2Q4 w - - 0 1",
        "k2q4/1q3q2/2q5/q5q1/4q3/1q4q1/PP1q4/KN6 b - - 0 1",
        "7k/8/8/3pP3/8/8/8/K7 w - d6 0 1",
        "7k/1q5p/5r1P/4Np2/R2p1P2/2bP2PK/1p6/1Q6 w - - 4 68",
        "7k/1q5p/7r/4Np2/R2p1P2/2bP2PK/Qp6/8 w - - 0 69",
        "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1"}) {
        auto generatedPosition=fromFen(fen,sideToMove);
        for(int color=1;color<=2;++color) {
            auto auditGenerator=[&](auto generate) {
                tmovelist first,second;int firstCount=0,secondCount=0;
                pas::fillChar(first,sizeof(first),0xa5);pas::fillChar(second,sizeof(second),0x5a);
                generate(first,firstCount);generate(second,secondCount);
                if(firstCount!=secondCount)generationDefinesEntries=false;
                for(int index=1;index<=std::min(firstCount,secondCount);++index)
                    if(first[index].move!=second[index].move || first[index].score!=second[index].score)generationDefinesEntries=false;
            };
            auditGenerator([&](tmovelist& list,int& count) {if(color==1)petir_movgen::generateWhiteCaptures(generatedPosition,list,count);else petir_movgen::generateBlackCaptures(generatedPosition,list,count);});
            auditGenerator([&](tmovelist& list,int& count) {if(color==1)petir_movgen::generateWhiteQuietMoves(generatedPosition,list,count);else petir_movgen::generateBlackQuietMoves(generatedPosition,list,count);});
            auditGenerator([&](tmovelist& list,int& count) {if(color==1)petir_movgen::generateWhiteCapturesAndPushes(generatedPosition,list,count);else petir_movgen::generateBlackCapturesAndPushes(generatedPosition,list,count);});
            auditGenerator([&](tmovelist& list,int& count) {if(color==1)petir_movgen::generateWhiteCapturesAndChecks(generatedPosition,list,count);else petir_movgen::generateBlackCapturesAndChecks(generatedPosition,list,count);});
            if(color==1?petir_tools::whiteInCheck(generatedPosition):petir_tools::blackInCheck(generatedPosition))
                auditGenerator([&](tmovelist& list,int& count) {int checks=0;if(color==1)petir_movgen::generateWhiteEvasions(generatedPosition,list,count,checks);else petir_movgen::generateBlackEvasions(generatedPosition,list,count,checks);});
        }
    }
    check(generationDefinesEntries,"generated move/score entries do not depend on initial buffer contents");
    // Freeze first-maximum selection and the swap order used by the old picker.
    tmovelist ordering;
    ordering[1]={11,5};ordering[2]={12,8};ordering[3]={13,8};ordering[4]={14,-1};ordering[5]={15,8};
    const std::array<int,5> selectedMoves{12,13,15,11,14};
    for(int current=1;current<=5;++current) {
        petir_next::selectBestMove(ordering,current,5);
        check(ordering[current].move==selectedMoves[current-1],"range-based move picker preserves maximum ties and swap order");
    }
    ordering[256]={23,std::numeric_limits<std::int32_t>::min()};
    petir_next::selectBestMove(ordering,256,256);
    check(ordering[256].move==23,"move picker accepts the last capacity slot");
    bool pickerOverflowRejected=false;
    try {petir_next::selectBestMove(ordering,1,257);}catch(const std::out_of_range&) {pickerOverflowRejected=true;}
    check(pickerOverflowRejected,"range-based move picker retains the unconditional capacity guard");
    // Baseline SEE values include losing quiet moves, recaptures, and an xray
    // attacker behind the moving rook. Both side conventions are intentional.
    struct SeeFixture {const char* fen;int from,to;std::array<int,2> expected;};
    for(const auto& fixture:std::array<SeeFixture,4>{{
        {"4k3/8/3p4/2p5/3P4/8/8/4K3 w - - 0 1",d4,c5,{0,1}},
        {"4r2k/8/8/4b3/8/8/4R3/K3Q3 w - - 0 1",e2,e5,{3,3}},
        {"7k/3p4/4q3/8/5N2/8/8/K7 w - - 0 1",f4,e6,{6,9}},
        {"7k/8/8/3p4/8/8/4Q3/K7 w - - 0 1",e2,e4,{-9,0}}
    }}) {
        auto exchange=fromFen(fixture.fen,sideToMove);const auto unchanged=exchange;
        for(int side=1;side<=2;++side)
            check(petir_usee::staticExchange(side,exchange,fixture.from,fixture.to)==fixture.expected[side-1]
                  && std::memcmp(&exchange,&unchanged,sizeof(exchange))==0,
                  "SEE workspace optimization preserves frozen exchange scores and input");
    }
    {
        // Search can capture a king without clearing its piece board. The
        // phantom king must still participate, valued by board as a pawn.
        auto exchange=fromFen("8/8/8/8/8/7k/6P1/K6R w - - 0 1",sideToMove);
        petir_makemove::makeWhiteMove(g2 | (h3 << 7),exchange);
        check(exchange.blackKingBoard==petir_bitboard_mask::squareBit(h3)
              && !(exchange.blackPieces & exchange.blackKingBoard) && exchange.board[h3]==whitePawn
              && petir_usee::staticExchange(whiteSide,exchange,h1,h2)==-5,
              "SEE preserves the phantom king recapture after a pseudo-legal king capture");
    }
    {
        // Independent ray walker: exercise all square pairs with deliberately
        // unrelated rotated occupancies and slider boards. It includes sliders
        // absent from occupancy and blockers absent from the slider boards.
        using namespace petir_bitboard_mask;
        auto walkedXray=[](std::uint64_t attack,int square,int target,const Position& xrayPosition) {
            const int fileDelta=(square & 7)-(target & 7),rankDelta=(square >> 3)-(target >> 3);
            if((fileDelta==0 && rankDelta==0) || (fileDelta!=0 && rankDelta!=0 && std::abs(fileDelta)!=std::abs(rankDelta)))return attack;
            const int fileStep=(fileDelta>0)-(fileDelta<0),rankStep=(rankDelta>0)-(rankDelta<0),step=fileStep+8*rankStep;
            const auto sliders=(fileDelta==0 || rankDelta==0)?xrayPosition.rooksAndQueens:xrayPosition.bishopsAndQueens;
            int file=(square & 7)+fileStep,rank=(square >> 3)+rankStep;
            while(file>=0 && file<8 && rank>=0 && rank<8) {
                const int behind=8*rank+file;
                attack|=squareBit(behind) & sliders;
                const bool occupied=rankDelta==0 ? (xrayPosition.allPieces & squareBit(behind))!=0
                    : fileDelta==0 ? (xrayPosition.allPiecesRotated90 & rotated90SquareMask[behind])!=0
                    : std::abs(step)==7 ? (xrayPosition.allPiecesA8H1 & rotatedA8H1SquareMask[behind])!=0
                    : (xrayPosition.allPiecesH8A1 & rotatedH8A1SquareMask[behind])!=0;
                if(occupied)break;
                file+=fileStep;rank+=rankStep;
            }
            return attack;
        };
        std::uint64_t seeSeed=UINT64_C(0x951aadfa20261011);
        auto nextBoard=[&]() {
            seeSeed^=seeSeed<<13;seeSeed^=seeSeed>>7;seeSeed^=seeSeed<<17;
            return seeSeed;
        };
        bool xraysMatch=true;
        for(int pattern=0;pattern<64;++pattern) {
            Position exchange{};
            exchange.allPieces=nextBoard();exchange.allPiecesRotated90=nextBoard();
            exchange.allPiecesA8H1=nextBoard();exchange.allPiecesH8A1=nextBoard();
            exchange.rooksAndQueens=nextBoard();exchange.bishopsAndQueens=nextBoard();
            if(pattern<2)exchange.allPieces=exchange.allPiecesRotated90=exchange.allPiecesA8H1=exchange.allPiecesH8A1
                =pattern==0?0:UINT64_MAX;
            if(pattern<4)exchange.rooksAndQueens=exchange.bishopsAndQueens=(pattern & 1)?UINT64_MAX:0;
            for(int target=0;target<64;++target)for(int square=0;square<64;++square) {
                const auto initial=nextBoard();auto actual=initial;
                petir_usee::addXray(actual,square,target,exchange);
                if(actual!=walkedXray(initial,square,target,exchange))xraysMatch=false;
            }
        }
        check(xraysMatch,"SEE x-rays match a ray walker for all directions and independent occupancy fields");
    }
    // Frozen forward-first order: pieces and targets nearest the enemy's back
    // rank come first, files a to h within a rank, and White mirrors Black.
    // The fixtures span several ranks and interleave two en passant sources
    // with ordinary pawn captures.
    for(const auto& fixture:std::array<std::array<const char*,3>,4>{{
        {{"4k3/2n5/8/6n1/8/1n6/8/7K b - - 0 1", "",
          "b3a1 b3c1 b3d2 b3d4 b3a5 b3c5 g5f3 g5h3 g5e4 g5e6 g5f7 g5h7 c7b5 c7d5 c7a6 c7e6 c7a8 e8d7 e8e7 e8f7 e8d8 e8f8 "}},
        {{"7k/8/1N6/8/6N1/8/2N5/4K3 w - - 0 1", "",
          "b6a8 b6c8 b6d7 b6d5 b6a4 b6c4 g4f6 g4h6 g4e5 g4e3 g4f2 g4h2 c2b4 c2d4 c2a3 c2e3 c2a1 e1d2 e1e2 e1f2 e1d1 e1f1 "}},
        {{"7k/8/8/8/pPp1pPp1/1R1B3R/8/7K b - f3 0 1",
          "c4b3 e4d3 a4b3 c4d3 g4h3 e4f3 g4f3 ",
          "a4a3 c4c3 e4e3 g4g3 h8g7 h8h7 h8g8 "}},
        {{"7k/8/1r1b3r/PpP1PpP1/8/8/8/7K w - f6 0 1",
          "c5b6 e5d6 a5b6 c5d6 g5h6 e5f6 g5f6 ",
          "a5a6 c5c6 e5e6 g5g6 h1g2 h1h2 h1g1 "}}
    }}) {
        auto ordered=fromFen(fixture[0],sideToMove);
        const bool isWhite=sideToMove==petir_header::whiteSide;
        for(int kind=0;kind<2;++kind) {
            tmovelist list;int count=0;
            if(kind==0) {
                if(isWhite)petir_movgen::generateWhiteCaptures(ordered,list,count);else petir_movgen::generateBlackCaptures(ordered,list,count);
            } else {
                if(isWhite)petir_movgen::generateWhiteQuietMoves(ordered,list,count);else petir_movgen::generateBlackQuietMoves(ordered,list,count);
            }
            std::string sequence;
            for(int itemIndex=1;itemIndex<=count;++itemIndex)sequence+=moveText(list[itemIndex].move,ordered,sideToMove)+" ";
            check(sequence==fixture[kind+1],"generator keeps the forward-first move order for both colours");
        }
    }
    struct PinFixture {const char* fen;std::uint64_t expected;};
    for(const auto& fixture:std::array<PinFixture,8>{{
        {"4r2k/8/8/8/8/8/4R3/4K3 w - - 0 1",UINT64_C(1)<<e2},
        {"4k3/4r3/8/8/8/8/8/4R2K b - - 0 1",UINT64_C(1)<<e7},
        {"k6b/8/5B2/8/3K4/8/8/8 w - - 0 1",UINT64_C(1)<<f6},
        {"4r2k/8/8/8/8/8/1R2p3/4K3 w - - 0 1",0},
        {"4r2k/8/8/8/8/4P3/4R3/4K3 w - - 0 1",0},
        {"5r1k/8/8/8/8/8/4R3/4K3 w - - 0 1",0},
        {"4b2k/8/8/8/8/8/4R3/4K3 w - - 0 1",0},
        {"4r2k/8/8/8/8/6b1/4RB2/4K3 w - - 0 1",(UINT64_C(1)<<e2)|(UINT64_C(1)<<f2)}
    }}) {
        auto pinned=fromFen(fixture.fen,sideToMove);const auto unchanged=pinned;
        check(petir_tools::pinnedPieces(pinned,sideToMove)==fixture.expected
              && std::memcmp(&pinned,&unchanged,sizeof(pinned))==0,
              "cached pin mask finds only single friendly blockers and preserves the position");
    }
    auto bare=fromFen("7k/8/8/8/8/8/8/K7 w - - 0 1",sideToMove);
    check(petir_ueval::evaluate(bare,sideToMove,-20500,20500)==0,"bare kings evaluate as draw");
    // Independent Syzygy WDL/DTZ fixtures. Mirror each across files and colors;
    // exercise full/lazy windows and opponentPerspective's reversed score perspective.
    struct KpkFixture { const char* name; const char* fen; int distanceToZero; };
    for (const auto& fixture : {
        KpkFixture{"opposition draw", "8/4k3/8/4K3/4P3/8/8/8 w - - 0 1", 0},
        KpkFixture{"opposition win", "8/4k3/8/4K3/4P3/8/8/8 b - - 0 1", 4},
        KpkFixture{"sixth-rank king", "4k3/8/4K3/4P3/8/8/8/8 w - - 0 1", 3},
        KpkFixture{"rook promotion avoids stalemate", "8/k1P5/2K5/8/8/8/8/8 w - - 0 1", 1},
        KpkFixture{"stalemate", "k7/2K5/1P6/8/8/8/8/8 b - - 0 1", 0},
        KpkFixture{"pawn can be captured", "8/8/8/4k3/4P3/8/8/K7 b - - 0 1", 0},
        KpkFixture{"blocked seventh-rank pawn wins", "4k3/4P3/4K3/8/8/8/8/8 w - - 0 1", 5},
        KpkFixture{"blocked seventh-rank pawn draws", "4k3/4P3/4K3/8/8/8/8/8 b - - 0 1", 0},
        KpkFixture{"distant king draw", "8/8/8/8/5k2/8/P7/K7 w - - 0 1", 0},
        KpkFixture{"rook pawn corner draw", "7k/8/6KP/8/8/8/8/8 w - - 0 1", 0},
        KpkFixture{"seventh-rank rook pawn draw", "k7/P7/1K6/8/8/8/8/8 w - - 0 1", 0},
        KpkFixture{"initial double push", "8/8/8/8/8/8/2P4k/K7 w - - 0 1", 1},
        KpkFixture{"defender delays pawn reset", "8/8/8/8/8/2P5/7k/K7 b - - 0 1", 14},
        KpkFixture{"king must unblock pawn", "7k/8/8/8/8/8/4P3/K7 w - - 0 1", 11}}) {
        int fixtureSide;
        const auto base = fromFen(fixture.fen,fixtureSide);
        const int pawnRank = std::countr_zero(base.whitePawns) / 8;
        for (bool flipColor : {false,true}) for (bool flipFile : {false,true}) {
            Position position{};
            for (int square = 0; square < 64; ++square)
                position.board[square ^ (flipColor ? 56 : 0) ^ (flipFile ? 7 : 0)]
                    = static_cast<std::int8_t>(base.board[square] * (flipColor ? -1 : 1));
            petir_customboard::rebuildBitboards(position);
            const int actualSide = flipColor ? 3-fixtureSide : fixtureSide;
            const std::string tag = std::string("KPK: ") + fixture.name
                + (flipColor ? " black pawn" : " white pawn") + (flipFile ? " reflected" : "");
            for (int clock : {0,79,80,96,97,98,99,100,255}) {
                position.fiftyMoveCounter = static_cast<std::uint8_t>(clock);
                const auto saved = position;
                const bool drawn = !fixture.distanceToZero || clock >= 100 || clock + fixture.distanceToZero > 100;
                int expected = drawn ? 0 : 700 + 8*pawnRank - fixture.distanceToZero;
                if (fixtureSide == blackSide) expected = -expected;
                int raw = 12345;
                check(petir_kpk::probe(position,actualSide,raw) && raw == expected,tag+" exact outcome and DTZ score");
                for (const auto& window : {std::array<int,2>{-20500,20500},{0,1},{1000,1001}}) {
                    check(petir_ueval::evaluate(position,actualSide,window[0],window[1]) == expected,
                        tag+" full/lazy score at halfmove "+std::to_string(clock));
                    check(petir_ueval::evaluate(position,3-actualSide,window[0],window[1],true) == -expected,
                        tag+" infhr perspective at halfmove "+std::to_string(clock));
                }
                check(std::memcmp(&position,&saved,sizeof(position)) == 0,tag+" leaves position unchanged");
            }
        }
    }
    // Draw contempt is returned as configured in both perspectives; a winning
    // score equal to contempt must still reverse sign under opponentPerspective.
    const int savedContempt = drawScore;
    auto kpkDraw = fromFen("8/4k3/8/4K3/4P3/8/8/8 w - - 0 1",sideToMove);
    auto kpkWin = fromFen("8/4k3/8/4K3/4P3/8/8/8 b - - 0 1",sideToMove);
    drawScore = -720;
    check(petir_ueval::evaluate(kpkDraw,1,0,1) == drawScore
        && petir_ueval::evaluate(kpkDraw,2,0,1,true) == drawScore,"KPK draws preserve contempt");
    check(petir_ueval::evaluate(kpkWin,2,0,1) == -720
        && petir_ueval::evaluate(kpkWin,1,0,1,true) == 720,"KPK winning score reverses even when equal to contempt");
    drawScore = savedContempt;
    int unusedKpkScore;
    auto notKpk = fromFen("7k/8/8/8/8/8/4P3/KQ6 w - - 0 1",sideToMove);
    check(!petir_kpk::probe(bare,1,unusedKpkScore)
        && !petir_kpk::probe(notKpk,1,unusedKpkScore),"KPK probe rejects other material");
    notKpk = fromFen("7k/8/8/8/8/8/3PP3/K7 w - - 0 1",sideToMove);
    check(!petir_kpk::probe(notKpk,1,unusedKpkScore),"KPK probe rejects two pawns");
    auto checkMirroredTable=[&](const char* name,const auto& white,const auto& black) {
        bool matches=true;
        for(int square=0;square<64;++square)
            if(white[square]!=black[square^56])matches=false;
        check(matches,std::string("piece-square table agrees after color reversal: ")+name);
    };
    checkMirroredTable("pawn",petir_ueval::whitePawnSquareBonus,petir_ueval::blackPawnSquareBonus);
    checkMirroredTable("king kingside endgame",petir_ueval::whiteKingEndgameKingsideBonus,petir_ueval::blackKingEndgameKingsideBonus);
    checkMirroredTable("king general endgame",petir_ueval::whiteKingEndgameBothWingsBonus,petir_ueval::blackKingEndgameBothWingsBonus);
    auto kingWhite=fromFen("k5r1/8/8/8/8/8/P6P/1R2K3 w - - 0 1",sideToMove);
    const int kingWhiteScore=petir_ueval::evaluate(kingWhite,sideToMove,-20500,20500);
    auto kingBlack=fromFen("1r2k3/p6p/8/8/8/8/8/K5R1 b - - 0 1",sideToMove);
    check(kingWhiteScore==petir_ueval::evaluate(kingBlack,sideToMove,-20500,20500),
        "endgame king placement agrees after color reversal");
    // Regressions for material scaling, king/passers and connected-pawn order.
    // Reflect ranks and colors; side-to-move scores must agree even when lazy
    // evaluation intentionally returns an approximation in a narrow window.
    struct SymmetryFixture {const char* name;const char* fen;};
    for (const auto& fixture : {
        SymmetryFixture{"minor and pawn versus bare king", "7k/8/8/3N4/4P3/8/8/4K3 w - - 0 1"},
        SymmetryFixture{"minor and pawn versus queen", "7k/8/8/3N4/4P3/8/8/q3K3 w - - 0 1"},
        SymmetryFixture{"minor and pawn versus rook", "7k/8/8/3N4/4P3/8/8/r3K3 w - - 0 1"},
        SymmetryFixture{"minor and pawn versus two knights", "6nk/8/8/3N4/4P3/8/8/n3K3 w - - 0 1"},
        SymmetryFixture{"nearby defending king", "8/1k1P4/8/8/8/7N/7K/8 b - - 0 1"},
        SymmetryFixture{"connected passers with promotion shortcut", "8/3b4/3k1p2/p4P2/P7/6pp/1b2K3/8 b - - 9 99"},
        SymmetryFixture{"staggered passers", "7k/6P1/5P2/8/8/8/8/R3K3 w - - 0 1"},
        SymmetryFixture{"doubled passers", "7k/8/5PP1/5P2/8/8/8/R3K3 w - - 0 1"}}) {
        int side=1;const auto original=fromFen(fixture.fen,side);
        for (int clock : {0,80,99}) {
            auto position=original;position.fiftyMoveCounter=static_cast<std::uint8_t>(clock);
            Position reversed{};
            for(int square=0;square<64;++square)
                reversed.board[square^56]=static_cast<std::int8_t>(-position.board[square]);
            petir_customboard::rebuildBitboards(reversed);reversed.fiftyMoveCounter=position.fiftyMoveCounter;
            const auto saved=position,savedReversed=reversed;
            const auto tag=std::string("evaluation symmetry: ")+fixture.name+" clock="+std::to_string(clock);
            const int full=petir_ueval::evaluate(position,side,-20500,20500);
            check(full==petir_ueval::evaluate(reversed,3-side,-20500,20500),tag+" full");
            for(const auto& window:{std::array<int,2>{0,1},{full,full+1}})
                check(petir_ueval::evaluate(position,side,window[0],window[1])
                    ==petir_ueval::evaluate(reversed,3-side,window[0],window[1]),tag+" narrow");
            check(petir_ueval::evaluate(position,3-side,-20500,20500,true)==-full
                &&petir_ueval::evaluate(reversed,side,-20500,20500,true)==-full,tag+" infhr");
            check(std::memcmp(&position,&saved,sizeof(position))==0
                &&std::memcmp(&reversed,&savedReversed,sizeof(reversed))==0,tag+" input unchanged");
        }
    }
    struct SymmetryScoreFixture {const char* name;const char* fen;int expected;};
    for(const auto& fixture:{
        SymmetryScoreFixture{"material scaling reference", "7k/8/8/3N4/4P3/8/8/4K3 w - - 0 1",536},
        SymmetryScoreFixture{"king distance reference", "8/1k1P4/8/8/8/7N/7K/8 b - - 0 1",-588},
        SymmetryScoreFixture{"connected shortcut reference", "8/3b4/3k1p2/p4P2/P7/6pp/1b2K3/8 b - - 9 99",1527}}) {
        auto position=fromFen(fixture.fen,sideToMove);
        check(petir_ueval::evaluate(position,sideToMove,-20500,20500)==fixture.expected,fixture.name);
    }
    // Expected chess classifications, independently specified rather than
    // obtained by copying the detector. Reuse pawn keys with different pieces
    // to ensure blockades are applied after probing the pawn-only cache.
    struct PotentialFixture {const char* name;const char* fen;int expected;};
    for (const auto& fixture : {
        PotentialFixture{"locked helper", "k7/6p1/8/6pP/6P1/8/8/K7 w - - 0 1",0},
        PotentialFixture{"current recapture defender", "k7/8/8/7p/6P1/7P/8/K7 w - - 0 1",92},
        PotentialFixture{"phalanx already supports push", "k7/8/8/7p/6PP/8/8/K7 w - - 0 1",92},
        PotentialFixture{"unsafe helper route", "k7/6p1/8/5p1P/8/6P1/8/K7 w - - 0 1",0},
        PotentialFixture{"doubled helpers are one resource", "k7/6p1/6p1/7P/6P1/6P1/8/K7 w - - 0 1",0},
        PotentialFixture{"shared passed helper discounted once", "7k/1p3p2/8/2P1P3/3P4/8/8/K7 w - - 0 1",39},
        PotentialFixture{"helper one rank ahead", "k7/7p/8/7P/6P1/8/8/K7 w - - 0 1",92},
        PotentialFixture{"helper ahead, push square attacked", "k7/8/7p/7P/6P1/8/8/K7 w - - 0 1",0},
        PotentialFixture{"clear candidate", "k7/6p1/8/7P/6P1/8/8/K7 w - - 0 1",117},
        PotentialFixture{"enemy knight blockade", "k7/6p1/7n/7P/6P1/8/8/K7 w - - 0 1",58},
        PotentialFixture{"friendly bishop blockade", "k7/6p1/7B/7P/6P1/8/8/K7 w - - 0 1",58},
        PotentialFixture{"clear candidate after cached blockade", "k7/6p1/8/7P/6P1/8/8/K7 w - - 0 1",117},
        PotentialFixture{"three-pawn breakthrough", "7k/ppp5/8/PPP5/8/8/8/7K w - - 0 1",16},
        PotentialFixture{"breakthrough stopped by extra pawn", "7k/pppp4/8/PPP5/8/8/8/7K w - - 0 1",0},
        PotentialFixture{"breakthrough gap occupied", "7k/ppp5/1n6/PPP5/8/8/8/7K w - - 0 1",0},
        PotentialFixture{"small candidate for rounding checks", "7k/8/8/8/3p4/8/2PP4/K7 w - - 98 1",53},
        PotentialFixture{"existing passer not counted again", "7k/8/8/4P3/8/8/8/K7 w - - 0 1",0}}) {
        auto position=fromFen(fixture.fen,sideToMove);
        const auto saved=position;
        const int cached=petir_ueval::candidatePawnPotential(position,true);
        check(cached==fixture.expected,std::string("pawn potential: ")+fixture.name);
        check(cached==petir_ueval::candidatePawnPotential(position,true,false),"pawn potential cache agrees with fresh calculation");
        Position reversed{};
        for(int square=0;square<64;++square)
            reversed.board[square^56]=static_cast<std::int8_t>(-position.board[square]);
        petir_customboard::rebuildBitboards(reversed);
        reversed.fiftyMoveCounter=position.fiftyMoveCounter;
        check(cached==petir_ueval::candidatePawnPotential(reversed,false),"pawn potential agrees after color reversal");
        check(petir_ueval::evaluate(position,sideToMove,-20500,20500)
            ==petir_ueval::evaluate(reversed,3-sideToMove,-20500,20500),"candidate full evaluation agrees after color reversal");
        check(std::memcmp(&position,&saved,sizeof(position))==0,"pawn potential leaves its input unchanged");
    }
    // More distinct pawn keys than table slots exercise eviction. Compare both
    // colors immediately and again after subsequent keys have replaced entries.
    std::uint64_t potentialRandom=UINT64_C(0x91b745ca6d2308ef);
    std::vector<Position> potentialBoards;
    bool potentialCacheMatches=true,potentialSymmetryMatches=true;
    for(int sample=0;sample<5000;++sample) {
        auto position=fromFen("7k/8/8/8/8/8/8/K7 w - - 0 1",sideToMove);
        for(int pawn=0;pawn<12;++pawn) {
            potentialRandom^=potentialRandom<<13;potentialRandom^=potentialRandom>>7;potentialRandom^=potentialRandom<<17;
            const int square=8+static_cast<int>(potentialRandom%48);
            position.board[square]=static_cast<std::int8_t>((potentialRandom>>8)&1?whitePawn:blackPawn);
        }
        petir_customboard::rebuildBitboards(position);
        Position reversed{};
        for(int square=0;square<64;++square)
            reversed.board[square^56]=static_cast<std::int8_t>(-position.board[square]);
        petir_customboard::rebuildBitboards(reversed);
        for(bool white:{false,true}) {
            const int fresh=petir_ueval::candidatePawnPotential(position,white,false);
            if(fresh!=petir_ueval::candidatePawnPotential(position,white))potentialCacheMatches=false;
            if(fresh!=petir_ueval::candidatePawnPotential(reversed,!white))potentialSymmetryMatches=false;
        }
        potentialBoards.push_back(position);
    }
    for(const auto& position:potentialBoards)for(bool white:{false,true})
        if(petir_ueval::candidatePawnPotential(position,white)!=petir_ueval::candidatePawnPotential(position,white,false))potentialCacheMatches=false;
    check(potentialCacheMatches,"pawn potential cache agrees across 5000 keys and revisits");
    check(potentialSymmetryMatches,"pawn potential agrees across 5000 color mirrors");
    auto potentialIntegration=fromFen("7k/8/8/8/3p4/8/2PP4/K7 w - - 0 1",sideToMove);
    check(petir_ueval::evaluate(potentialIntegration,sideToMove,-20500,20500)==348,"candidate raw bonus enters full evaluation");
    potentialIntegration.fiftyMoveCounter=98;
    check(petir_ueval::evaluate(potentialIntegration,sideToMove,-20500,20500)==52,"candidate bonus is scaled and rounded with the whole evaluation");
    struct ConnectedFixture {const char* name;const char* fen;int expected;};
    for(const auto& fixture:{
        ConnectedFixture{"single pawn", "7k/8/5P2/8/8/8/8/R3K3 w - - 0 1",0},
        ConnectedFixture{"sixth rank pair counted once", "7k/8/5PP1/8/8/8/8/R3K3 w - - 0 1",114},
        ConnectedFixture{"staggered sixth and seventh", "7k/6P1/5P2/8/8/8/8/R3K3 w - - 0 1",56},
        ConnectedFixture{"fifth rank pair", "7k/8/8/5PP1/8/8/8/R3K3 w - - 0 1",66},
        ConnectedFixture{"staggered fifth and sixth", "7k/8/6P1/5P2/8/8/8/R3K3 w - - 0 1",41},
        ConnectedFixture{"widely staggered passers", "7k/6P1/8/8/5P2/8/8/R3K3 w - - 0 1",5},
        ConnectedFixture{"three neighboring passers", "7k/8/4PPP1/8/8/8/8/R3K3 w - - 0 1",228},
        ConnectedFixture{"doubled pawn not counted again", "7k/8/5PP1/5P2/8/8/8/R3K3 w - - 0 1",114},
        ConnectedFixture{"enemy pawn prevents passers", "7k/6p1/5PP1/8/8/8/8/R3K3 w - - 0 1",0}}) {
        int side=1;const auto base=fromFen(fixture.fen,side);
        for(bool colorFlip:{false,true})for(bool fileFlip:{false,true}) {
            Position position{};
            for(int square=0;square<64;++square)
                position.board[square^(colorFlip?56:0)^(fileFlip?7:0)]
                    =static_cast<std::int8_t>(base.board[square]*(colorFlip?-1:1));
            petir_customboard::rebuildBitboards(position);const auto saved=position;
            check(petir_ueval::connectedPassedPawns(position,!colorFlip)==fixture.expected,
                std::string("connected passers: ")+fixture.name);
            check(std::memcmp(&position,&saved,sizeof(position))==0,"connected passers leave input unchanged");
        }
    }
    // The evaluation finds passed pawns for the whole board at once with file
    // fills. That equals the per-pawn mask test only if each passed-pawn mask holds
    // exactly the squares in front of the pawn on its own and the neighbouring files.
    bool passedMasksMatch=true,fileMasksMatch=true;
    for(int square=0;square<64;++square)for(int other=0;other<64;++other) {
        const bool nearFile=std::abs((square&7)-(other&7))<=1;
        const auto otherBit=petir_bitboard_mask::squareBit(other);
        if(((petir_bitboard_mask::whitePassedPawnMask[square]&otherBit)!=0)!=(nearFile && other/8>square/8))passedMasksMatch=false;
        if(((petir_bitboard_mask::blackPassedPawnMask[square]&otherBit)!=0)!=(nearFile && other/8<square/8))passedMasksMatch=false;
    }
    for(int file=0;file<8;++file)if(petir_bitboard_mask::fileMask[file]!=(0x0101010101010101ULL<<file))fileMasksMatch=false;
    check(passedMasksMatch,"passed-pawn masks hold the squares in front on the pawn's and the neighbouring files");
    check(fileMasksMatch && petir_bitboard_mask::notFileAMask==~petir_bitboard_mask::fileMask[0]
        && petir_bitboard_mask::notFileHMask==~petir_bitboard_mask::fileMask[7],"file masks match the board geometry");
    // Exercise the new component independently of material/PST/draw scaling.
    struct PasserActivityFixture {const char* name;const char* fen;int pawn;int expected;};
    for(const auto& fixture:{
        PasserActivityFixture{"clear path, supporting king", "n6k/8/8/3KP3/8/8/8/1R6 w - - 0 1",e5,34},
        PasserActivityFixture{"own king far away", "n6k/8/8/4P3/8/8/8/KR6 w - - 0 1",e5,10},
        PasserActivityFixture{"defender king controls push", "n7/5k2/8/3KP3/8/8/8/1R6 w - - 0 1",e5,3},
        PasserActivityFixture{"enemy blockade", "7k/8/4n3/3KP3/8/8/8/1R6 w - - 0 1",e5,20},
        PasserActivityFixture{"friendly blockade", "n6k/8/4B3/3KP3/8/8/8/1R6 w - - 0 1",e5,20},
        PasserActivityFixture{"knight controls push", "7k/6n1/8/3KP3/8/8/8/1R6 w - - 0 1",e5,15},
        PasserActivityFixture{"bishop controls push", "7k/8/8/3KP3/6b1/8/8/1R6 w - - 0 1",e5,15},
        PasserActivityFixture{"rook controls push", "7k/8/7r/3KP3/8/8/8/1R6 w - - 0 1",e5,15},
        PasserActivityFixture{"queen controls push", "7k/8/7q/3KP3/8/8/8/1R6 w - - 0 1",e5,15},
        PasserActivityFixture{"rook revealed behind pawn", "7k/8/8/3KP3/8/8/4r3/1R6 w - - 0 1",e5,15},
        PasserActivityFixture{"queen revealed behind pawn", "7k/8/8/3KP3/8/8/4q3/1R6 w - - 0 1",e5,15},
        PasserActivityFixture{"later path square attacked", "6rk/8/8/3KP3/8/8/8/1R6 w - - 0 1",e5,32},
        PasserActivityFixture{"later path square occupied", "n6k/4B3/8/3KP3/8/8/8/1R6 w - - 0 1",e5,32},
        PasserActivityFixture{"seventh-rank board edge", "n6k/4P3/3K4/8/8/8/8/1R6 w - - 0 1",e7,84},
        PasserActivityFixture{"fourth-rank pawn excluded", "n6k/8/8/3K4/4P3/8/8/1R6 w - - 0 1",e4,0},
        PasserActivityFixture{"middlegame excluded", "rq5k/8/8/3KP3/8/8/8/1R6 w - - 0 1",e5,0},
        PasserActivityFixture{"KPK excluded", "7k/8/8/3KP3/8/8/8/8 w - - 0 1",e5,0}}) {
        auto position=fromFen(fixture.fen,sideToMove);const auto saved=position;
        const auto tag=std::string("passed-pawn activity: ")+fixture.name;
        check(isConsistent(position),tag+" consistent position");
        check(petir_ueval::passedPawnActivity(position,fixture.pawn,true)==fixture.expected,tag);
        // Reverse colors/ranks, including slider occupancy and all piece types.
        std::string placement;int empty=0;
        for(int square=63;square>=0;--square) {
            const int source=(square^7)^56;
            const int piece=-position.board[source];
            if(!piece)++empty;
            else {if(empty)placement+=char('0'+empty);empty=0;
                const char* symbols=" PNBRQK";char symbol=symbols[std::abs(piece)];
                placement+=piece>0?symbol:char(symbol-'A'+'a');}
            if((square&7)==0) {if(empty)placement+=char('0'+empty);empty=0;if(square)placement+='/';}
        }
        int reversedSide=2;auto reversed=fromFen(placement+" b - - 0 1",reversedSide);
        check(petir_ueval::passedPawnActivity(reversed,fixture.pawn^56,false)==fixture.expected,
              tag+" color reversal");
        check(std::memcmp(&position,&saved,sizeof(position))==0,tag+" input unchanged");
    }
    // Endgame activity: safety, pawn targets, passive defense and stable blockades.
    struct EndgameActivityFixture {const char* name;const char* fen;int square;int expected;};
    for(const auto& fixture:{
        EndgameActivityFixture{"rook free movement", "7k/8/8/8/8/8/7P/R6K w - - 0 1",a1,-12},
        EndgameActivityFixture{"rook attacks weak pawn", "7k/8/8/R3p3/8/8/7P/7K w - - 0 1",a5,10},
        EndgameActivityFixture{"rook attacks defended pawn", "7k/8/5p2/R3p3/8/8/7P/7K w - - 0 1",a5,-12},
        EndgameActivityFixture{"rook under attack", "7k/8/2n5/R3p3/8/8/7P/7K w - - 0 1",a5,0},
        EndgameActivityFixture{"restricted rook tied to pawn", "7k/8/8/8/8/8/P1r5/RN5K w - - 0 1",a1,-12},
        EndgameActivityFixture{"restricted rook pawn has another defender", "7k/8/8/8/8/8/P1r5/RB5K w - - 0 1",a1,12},
        EndgameActivityFixture{"rook pawnless excluded", "7k/8/8/8/8/8/8/R6K w - - 0 1",a1,0},
        EndgameActivityFixture{"rook middlegame excluded", "rq5k/8/8/8/8/8/7P/R6K w - - 0 1",a1,0},
        EndgameActivityFixture{"knight supported useful outpost", "7k/8/1p6/3N4/4P3/8/8/4K3 w - - 0 1",d5,19},
        EndgameActivityFixture{"knight no support", "7k/8/1p6/3N4/8/4P3/8/4K3 w - - 0 1",d5,0},
        EndgameActivityFixture{"knight defended target", "8/k7/1p6/3N4/4P3/8/8/4K3 w - - 0 1",d5,0},
        EndgameActivityFixture{"knight future pawn challenge", "7k/4p3/1p6/3N4/4P3/8/8/4K3 w - - 0 1",d5,0},
        EndgameActivityFixture{"knight supported blockade", "7k/8/3p4/3N4/4P3/8/8/4K3 w - - 0 1",d5,28},
        EndgameActivityFixture{"knight no concrete target", "7k/8/7p/3N4/4P3/8/8/4K3 w - - 0 1",d5,0},
        EndgameActivityFixture{"knight under attack", "7k/2n5/1p6/3N4/4P3/8/8/4K3 w - - 0 1",d5,0},
        EndgameActivityFixture{"knight edge blockade", "7k/8/p7/N7/1P6/8/8/4K3 w - - 0 1",a5,0},
        EndgameActivityFixture{"KPK excluded", "7k/8/8/3KP3/8/8/8/8 w - - 0 1",d5,0}}) {
        auto position=fromFen(fixture.fen,sideToMove);const auto saved=position;
        const auto tag=std::string("endgame piece activity: ")+fixture.name;
        check(isConsistent(position),tag+" consistent position");
        const int activity=petir_ueval::endgamePieceActivity(position,fixture.square,true);
        check(activity==fixture.expected,tag+" actual="+std::to_string(activity));
        std::string placement;int empty=0;
        for(int square=63;square>=0;--square) {
            const int piece=-position.board[(square^7)^56];
            if(!piece)++empty;
            else {if(empty)placement+=char('0'+empty);empty=0;
                const char* symbols=" PNBRQK";char symbol=symbols[std::abs(piece)];
                placement+=piece>0?symbol:char(symbol-'A'+'a');}
            if((square&7)==0) {if(empty)placement+=char('0'+empty);empty=0;if(square)placement+='/';}
        }
        int reversedSide=2;auto reversed=fromFen(placement+" b - - 0 1",reversedSide);
        check(petir_ueval::endgamePieceActivity(reversed,fixture.square^56,false)==activity,
              tag+" color reversal");
        check(petir_ueval::evaluate(position,1,-20500,20500)
              ==petir_ueval::evaluate(reversed,2,-20500,20500),tag+" full evaluation color reversal");
        check(std::memcmp(&position,&saved,sizeof(position))==0,tag+" input unchanged");
    }
    // KRP-KR soft scaling: guarded defenses, favorable seventh-rank pawns,
    // side-to-move sensitivity and both board symmetries.
    struct RookPawnFixture {const char* name;const char* fen;int scale;};
    for(const auto& fixture:{
        RookPawnFixture{"Philidor", "8/4k3/r7/4K3/4P3/8/8/1R6 w - - 0 1",25},
        RookPawnFixture{"rear checks", "1k6/3K4/1P6/8/1R6/8/8/7r w - - 0 1",25},
        RookPawnFixture{"rook pawn trapped rook", "R7/P6k/8/8/8/r7/5K2/8 w - - 0 1",25},
        RookPawnFixture{"seventh rank", "2K5/k1P5/8/8/8/8/7r/3R4 w - - 0 1",100},
        RookPawnFixture{"defender to move fallback", "2K5/k1P5/8/8/8/8/7r/3R4 b - - 0 1",50},
        RookPawnFixture{"rook capture tactic", "8/4k3/r7/4K3/4P3/8/8/R7 w - - 0 1",50},
        RookPawnFixture{"Philidor winning exception", "3k4/8/1r6/8/3K4/2P5/3R4/8 w - - 0 1",50},
        RookPawnFixture{"unrecognized position", "7k/7r/8/8/8/8/7P/R6K w - - 0 1",50},
        RookPawnFixture{"KPK outside scope", "7k/8/8/3KP3/8/8/8/8 w - - 0 1",100},
        RookPawnFixture{"extra pawn outside scope", "8/4k3/r7/4K3/3PP3/8/8/1R6 w - - 0 1",100},
        RookPawnFixture{"extra minor outside scope", "8/4k3/r7/4K3/4P3/8/8/1RN5 w - - 0 1",100}}) {
        int originalSide=1;auto original=fromFen(fixture.fen,originalSide);
        for(bool colorFlip:{false,true})for(bool fileFlip:{false,true}) {
            std::string placement;int empty=0;
            const int flip=(colorFlip?56:0)^(fileFlip?7:0);
            for(int square=63;square>=0;--square) {
                int piece=original.board[(square^7)^flip];if(colorFlip)piece=-piece;
                if(!piece)++empty;
                else {if(empty)placement+=char('0'+empty);empty=0;
                    const char* symbols=" PNBRQK";char symbol=symbols[std::abs(piece)];
                    placement+=piece>0?symbol:char(symbol-'A'+'a');}
                if((square&7)==0){if(empty)placement+=char('0'+empty);empty=0;if(square)placement+='/';}
            }
            int side=colorFlip?3-originalSide:originalSide;
            auto position=fromFen(placement+(side==1?" w - - 0 1":" b - - 0 1"),side);
            const auto saved=position;
            const auto tag=std::string("rook-pawn scaling: ")+fixture.name
                +" color="+std::to_string(colorFlip)+" file="+std::to_string(fileFlip);
            check(isConsistent(position),tag+" consistent position");
            check(petir_ueval::rookPawnScale(position,side)==fixture.scale,tag);
            const int score=petir_ueval::evaluate(position,side,-20500,20500);
            check(score==-petir_ueval::evaluate(position,3-side,-20500,20500,true),
                  tag+" infhr preserves actual side to move");
            check(score==petir_ueval::evaluate(position,side,score,score+1),tag+" narrow window");
            check(std::memcmp(&position,&saved,sizeof(position))==0,tag+" input unchanged");
        }
    }
    {
        int side=1;auto philidor=fromFen("8/4k3/r7/4K3/4P3/8/8/1R6 w - - 0 1",side);
        auto seventh=fromFen("2K5/k1P5/8/8/8/8/7r/3R4 w - - 0 1",side);
        check(petir_ueval::evaluate(philidor,side,-20500,20500)==45,"Philidor full score uses 25 percent scaling");
        check(petir_ueval::evaluate(seventh,side,-20500,20500)==373,"seventh-rank full score retains full advantage");
    }
    // A king one file from the black passer must get the same evaluation
    // adjustment as in the color-reversed position (d3/e1 becomes d6/e8).
    auto blackPasser=fromFen("k7/8/8/8/8/3p4/8/1R2K3 w - - 0 1",sideToMove);
    const int blackPasserScore=petir_ueval::evaluate(blackPasser,sideToMove,-20500,20500);
    auto whitePasser=fromFen("1r2k3/8/3P4/8/8/8/8/K7 b - - 0 1",sideToMove);
    check(blackPasserScore==petir_ueval::evaluate(whitePasser,sideToMove,-20500,20500),
        "passed-pawn defender king file distance agrees after color reversal");
    auto isolatedWhite=fromFen("7k/8/8/8/8/4P3/4P3/KR5r w - - 0 1",sideToMove);
    const int isolatedWhiteScore=petir_ueval::evaluate(isolatedWhite,sideToMove,-20500,20500);
    auto isolatedBlack=fromFen("kr5R/4p3/4p3/8/8/8/8/7K b - - 0 1",sideToMove);
    check(isolatedWhiteScore==petir_ueval::evaluate(isolatedBlack,sideToMove,-20500,20500),
        "isolated doubled pawns receive the same penalty after color reversal");
    // With e5/e6 doubled passers, only the front pawn's advance bonus counts.
    // Probe each rank's bonus independently without freezing the whole score.
    auto doubledPassers=fromFen("7k/8/4P3/4P3/8/8/8/KR6 w - - 0 1",sideToMove);
    const int doubledPassersScore=petir_ueval::evaluate(doubledPassers,sideToMove,-20500,20500);
    int doubledMirrorSide=2;
    auto doubledMirror=fromFen("kr6/8/8/8/4p3/4p3/8/7K b - - 0 1",doubledMirrorSide);
    check(doubledPassersScore==petir_ueval::evaluate(doubledMirror,doubledMirrorSide,-20500,20500),
        "doubled passer evaluation agrees after color reversal");
    for(int rank:{4,5}) {
        const auto bonus=petir_ueval::whitePassedPawnRankBonus[rank];
        petir_ueval::whitePassedPawnRankBonus[rank]=static_cast<std::uint8_t>(bonus+1);
        const int adjustedScore=petir_ueval::evaluate(doubledPassers,sideToMove,-20500,20500);
        petir_ueval::whitePassedPawnRankBonus[rank]=bonus;
        check(adjustedScore-doubledPassersScore==(rank==5?1:0),
            std::string("white doubled passers use the front pawn's advance bonus: ")+(rank==5?"e6":"e5"));
    }
    // Each bishop has four own pawns on its color and no opposing bishop.
    // Neutralizing just that penalty isolates its effect from other terms.
    struct BadBishopFixture {const char* name;const char* fen;int sign;};
    for(const auto& fixture:{
        BadBishopFixture{"black", "7k/8/8/3p1p2/2p1p3/8/8/KR3b2 w - - 0 1",1},
        BadBishopFixture{"white", "7k/8/8/3P1P2/2P1P3/8/8/KB5r w - - 0 1",-1}}) {
        auto position=fromFen(fixture.fen,sideToMove);
        const int score=petir_ueval::evaluate(position,sideToMove,-20500,20500);
        const auto penalty=petir_evalmask::badBishopPenalty[4];
        petir_evalmask::badBishopPenalty[4]=0;
        const int neutralScore=petir_ueval::evaluate(position,sideToMove,-20500,20500);
        petir_evalmask::badBishopPenalty[4]=penalty;
        check(score-neutralScore==fixture.sign*penalty,
            std::string("bad-bishop penalty applies without an opposing bishop: ")+fixture.name);
    }
    // Isolate king pressure from the mobility/PST effects of moving the rook.
    auto kingPressureContribution=[&](const char* fen) {
        int side=1;auto position=fromFen(fen,side);
        const int score=petir_ueval::evaluate(position,side,-20500,20500);
        const auto pressureTable=petir_ueval::kingPressurePenalty;
        for(int index=0;index<64;++index)petir_ueval::kingPressurePenalty[index]=0;
        const int neutralScore=petir_ueval::evaluate(position,side,-20500,20500);
        petir_ueval::kingPressurePenalty=pressureTable;
        return score-neutralScore;
    };
    struct RookDefenseFixture {const char* name;const char* defended;const char* exposed;};
    for(const auto& fixture:{
        RookDefenseFixture{"black",
            "r1b3rk/4Q3/4N3/8/8/8/7P/R3K2R w - - 0 1",
            "r1b2r1k/4Q3/4N3/8/8/8/7P/R3K2R w - - 0 1"},
        RookDefenseFixture{"white",
            "r3k2r/7p/8/8/8/4n3/4q3/R1B3RK b - - 0 1",
            "r3k2r/7p/8/8/8/4n3/4q3/R1B2R1K b - - 0 1"}}) {
        check(kingPressureContribution(fixture.defended)<kingPressureContribution(fixture.exposed),
            std::string("rook defense reduces king danger without its own queen: ")+fixture.name);
    }
    // These windows pass stage 1; the removed later lazy checkpoints used to
    // return before king-danger evaluation or rule-50 scaling completed.
    struct LazyEvalFixture {const char* name;const char* fen;int alpha,beta,score;};
    for(const auto& fixture:{
        LazyEvalFixture{"king danger after former stage 2",
            "8/4n1r1/1p3P2/p1k5/P1B5/1P2bK1N/2PN2Pq/5RR1 w - - 0 1",-68,-67,-294},
        LazyEvalFixture{"rule-50 scaling after former stage 3",
            "7k/8/8/8/8/8/8/KQ6 w - - 99 1",200,201,107}}) {
        auto position=fromFen(fixture.fen,sideToMove);
        check(isConsistent(position),std::string("lazy evaluation fixture is consistent: ")+fixture.name);
        const int full=petir_ueval::evaluate(position,sideToMove,-20500,20500);
        const int narrow=petir_ueval::evaluate(position,sideToMove,fixture.alpha,fixture.beta);
        check(full==fixture.score,std::string("full evaluation reference: ")+fixture.name+" actual="+std::to_string(full));
        check(narrow==full,std::string("narrow evaluation completes remaining terms: ")+fixture.name);
    }
    // Use windows that previously let stage 1 bypass the final draw scaling.
    for(const char* fen:{
        "7k/8/8/8/8/8/8/KQ6 w - - 0 1",
        "kq6/8/8/8/8/8/8/7K b - - 0 1"}) {
        int strongSide=1;auto position=fromFen(fen,strongSide);
        for(int counter:{80,99})for(int side:{strongSide,3-strongSide}) {
            position.fiftyMoveCounter=static_cast<std::uint8_t>(counter);
            const int full=petir_ueval::evaluate(position,side,-20500,20500);
            const int beta=side==strongSide?101:-100;
            const int narrow=petir_ueval::evaluate(position,side,beta-1,beta);
            check(narrow==full,"stage 1 preserves rule-50 scaling at halfmove "+std::to_string(counter)
                +" / side "+std::to_string(side)+" / stronger side "+std::to_string(strongSide));
        }
    }
    for(const char* fen:{
        "7k/8/8/8/8/8/8/KRB4r w - - 0 1",
        "krb4R/8/8/8/8/8/8/7K b - - 0 1",
        "7k/8/8/8/8/8/8/KRN4r w - - 0 1",
        "krn4R/8/8/8/8/8/8/7K b - - 0 1"}) {
        int strongSide=1;auto position=fromFen(fen,strongSide);
        for(int side:{strongSide,3-strongSide}) {
            const int full=petir_ueval::evaluate(position,side,-20500,20500);
            const int beta=side==strongSide?-999:1000;
            const int narrow=petir_ueval::evaluate(position,side,beta-1,beta);
            check(narrow==full,"stage 1 preserves rook-plus-minor draw scaling: "+std::string(fen)
                +" / side "+std::to_string(side));
        }
    }
    check(insufficientMaterial(bare),"bare kings are insufficient mating material");
    for(const char* fen:{
        "7k/8/8/8/8/8/8/KN6 w - - 0 1",
        "7k/8/8/8/8/8/8/KB6 w - - 0 1",
        "7k/8/8/4b3/8/8/8/K1B5 w - - 0 1"}) {
        auto dead=fromFen(fen,sideToMove);
        check(insufficientMaterial(dead),"single minor / same-color bishops cannot mate");
    }
    for(const char* fen:{
        "7k/8/8/8/8/8/8/KNN5 w - - 0 1",
        "6nk/8/8/8/8/8/8/KN6 w - - 0 1",
        "7k/8/8/3b4/8/8/8/K1B5 w - - 0 1"}) {
        auto live=fromFen(fen,sideToMove);
        check(!insufficientMaterial(live),"do not claim insufficient material when mate is possible");
    }
    const std::string fenState="r3k2r/8/8/3pP3/8/8/8/R3K2R w KQkq d6 42 17";
    int fullMoveNumber=0;auto roundTrip=fromFen(fenState,sideToMove,&fullMoveNumber);
    check(toFen(roundTrip,sideToMove,fullMoveNumber)==fenState,"FEN output preserves castling, en passant, and counters");
    const auto oldMode=timeMode,oldMoves=gamePlyCount,oldRmoves=movesPerTimeControl,oldClock=whiteClock;
    timeMode=movesInMinutes;movesPerTimeControl=40;whiteClock=2400;
    gamePlyCount=0;const int periodBudget=originalTimeBudget(1);
    gamePlyCount=80;check(originalTimeBudget(1)==periodBudget,"clock budget resets at the second time-control period");
    gamePlyCount=160;check(originalTimeBudget(1)==periodBudget,"clock budget resets at later time-control periods");
    gamePlyCount=78;check(originalTimeBudget(1)==800,"last move of a time-control period gets the legacy budget");
    timeMode=oldMode;gamePlyCount=oldMoves;movesPerTimeControl=oldRmoves;whiteClock=oldClock;
    auto enPassantPosition=fromFen("7k/8/8/3pP3/8/8/8/K7 w - d6 0 1",sideToMove);
    int enPassantMove=parseMove("e5d6",enPassantPosition,sideToMove);check(enPassantMove!=0,"en passant parsed");makeMove(enPassantPosition,sideToMove,enPassantMove);
    check(enPassantPosition.board[35]==0 && enPassantPosition.board[43]==1 && isConsistent(enPassantPosition),"en passant updates occupancy, material, and hash");
    // An en passant evasion must remove the check without exposing another
    // attack. Also cover legal captures and interpositions for both colors.
    for(const auto& [fen,ep_legal]:std::initializer_list<std::pair<const char*,bool>>{
        {"4r2k/8/8/3pP3/4K3/8/8/8 w - d6 0 1",false},
        {"8/8/8/4k3/3Pp3/8/8/4R2K b - d3 0 1",false},
        {"1r5k/8/8/pP6/1K6/8/8/8 w - a6 0 1",false},
        {"8/8/8/1k6/Pp6/8/8/1R5K b - a3 0 1",false},
        {"6rk/8/8/6Pp/6K1/8/8/8 w - h6 0 1",false},
        {"8/8/8/6k1/6pP/8/8/6RK b - h3 0 1",false},
        {"7k/8/8/3pP3/4K3/8/8/8 w - d6 0 1",true},
        {"8/8/8/4k3/3Pp3/8/8/7K b - d3 0 1",true},
        {"7k/8/8/2Pp1p2/2K5/8/8/8 w - f6 0 1",false},
        {"8/8/8/2k5/2pP1P2/8/8/7K b - f3 0 1",false},
        {"1b5k/8/8/2Pp4/8/8/7K/8 w - d6 0 1",true},
        {"8/7k/8/8/2pP4/8/8/1B5K b - d3 0 1",true}}) {
        auto evasion=fromFen(fen,sideToMove);const auto originalFen=toFen(evasion,sideToMove,1);
        check(sideToMove==1?petir_tools::whiteInCheck(evasion):petir_tools::blackInCheck(evasion),"en passant evasion fixture starts in check");
        tmovelist candidates;int count=0,checkCount=0;
        if(sideToMove==1)petir_movgen::generateWhiteEvasions(evasion,candidates,count,checkCount);else petir_movgen::generateBlackEvasions(evasion,candidates,count,checkCount);
        std::vector<int> actual,expected;bool containsEp=false;
        for(int itemIndex=1;itemIndex<=count;++itemIndex) {
            actual.push_back(candidates[itemIndex].move);
            if(((candidates[itemIndex].move>>7)&127)==enPassantTarget)containsEp=true;
        }
        auto appendLegal=[&] {
            for(int index=1;index<=count;++index) {
                auto child=evasion;makeMove(child,sideToMove,candidates[index].move);
                if(!(sideToMove==1?petir_tools::whiteInCheck(child):petir_tools::blackInCheck(child)))expected.push_back(candidates[index].move);
            }
        };
        if(sideToMove==1)petir_movgen::generateWhiteCaptures(evasion,candidates,count);else petir_movgen::generateBlackCaptures(evasion,candidates,count);appendLegal();
        if(sideToMove==1)petir_movgen::generateWhiteQuietMoves(evasion,candidates,count);else petir_movgen::generateBlackQuietMoves(evasion,candidates,count);appendLegal();
        std::sort(actual.begin(),actual.end());std::sort(expected.begin(),expected.end());
        check(containsEp==ep_legal,"en passant evasion legality "+std::string(fen));
        check(actual==expected,"en passant evasions match independent king-safety filtering "+std::string(fen));
        check(toFen(evasion,sideToMove,1)==originalFen && isConsistent(evasion),"en passant evasion generation preserves the position");
    }
    for(const char* fen:{
        "3rr2k/8/8/3pPP2/3PKP2/3PPP2/8/8 w - d6 0 1",
        "8/8/3ppp2/3pkp2/3Ppp2/8/8/3RR2K b - d3 0 1"}) {
        auto epMate=fromFen(fen,sideToMove);tmovelist candidates;int count=0,checkCount=0;
        if(sideToMove==1)petir_movgen::generateWhiteEvasions(epMate,candidates,count,checkCount);else petir_movgen::generateBlackEvasions(epMate,candidates,count,checkCount);
        check(legalMoves(epMate,sideToMove).empty(),"illegal en passant is the only pseudo-evasion in a checkmate fixture");
        prepareSearch(sideToMove,false,0);useWinboard=false;
        check(count==0 && petir_search::quiescenceSearch(sideToMove,-20500,20500,epMate,1,2,0,petir_search::noValue,petir_search::pvNode)==-19998,"quiescence recognizes mate when en passant would expose the king");
    }
    for(const char* fen:{
        "7k/5Q2/6K1/8/8/8/8/8 b - - 0 1",
        "8/8/8/8/8/6k1/5q2/7K w - - 0 1",
        "R5bk/5Q2/6K1/8/8/8/8/8 b - - 0 1",
        "8/8/8/8/8/6k1/5q2/r5BK w - - 0 1",
        "8/8/1b6/8/8/5k2/r7/6nK w - - 0 1",
        "6Nk/R7/5K2/8/8/1B6/8/8 b - - 0 1",
        "7k/5Q2/6K1/8/8/8/p7/N7 b - - 0 1",
        "n7/P7/8/8/8/6k1/5q2/7K w - - 0 1",
        "8/1b6/8/8/5b2/4k3/5qP1/7K w - - 0 1",
        "7k/5Qp1/4K3/5B2/8/8/1B6/8 b - - 0 1"}) {
        auto stale=fromFen(fen,sideToMove);const auto originalFen=toFen(stale,sideToMove,1);
        check(!(sideToMove==1?petir_tools::whiteInCheck(stale):petir_tools::blackInCheck(stale)) && legalMoves(stale,sideToMove).empty(),"quiescence fixture is stalemate "+std::string(fen));
        prepareSearch(sideToMove,false,0);useWinboard=false;
        check(petir_search::quiescenceSearch(sideToMove,-20500,20500,stale,1,2,0,petir_search::noValue,petir_search::pvNode)==drawScore,"quiescence scores stalemate as a draw with checks enabled");
        check(petir_search::quiescenceSearch(sideToMove,-1,0,stale,0,2,0,petir_search::noValue,petir_search::cutNode)==drawScore,"capture-only quiescence scores stalemate as a draw");
        check(petir_search::quiescenceSearch(sideToMove,320,321,stale,0,2,1,321,petir_search::cutNode)==drawScore,"stalemate overrides a cached stand-pat beta cutoff");
        check(petir_search::quiescenceSearch(sideToMove,-20500,20500,stale,1,63,0,petir_search::noValue,petir_search::pvNode)==drawScore,"stalemate overrides the quiescence depth-limit evaluation");
        check(toFen(stale,sideToMove,1)==originalFen && isConsistent(stale) && nodeCount==0,"stalemate probes preserve the position and search node count");
    }
    for(const char* fen:{
        "7k/4Q3/6K1/8/8/8/8/8 b - - 0 1",
        "8/8/8/8/8/6k1/4q3/7K w - - 0 1",
        "8/8/8/8/8/5k2/r7/6nK w - - 0 1",
        "6Nk/R7/5K2/8/8/8/8/8 b - - 0 1",
        "8/8/8/8/8/6k1/5q2/1N5K w - - 0 1",
        "8/P7/8/8/8/6k1/5q2/7K w - - 0 1",
        "8/8/2p5/2Pp4/8/6k1/5q2/7K w - d6 0 1",
        "7k/5Q2/6K1/8/2pP4/2P5/8/8 b - d3 0 1"}) {
        auto live=fromFen(fen,sideToMove);
        check(!(sideToMove==1?petir_tools::whiteInCheck(live):petir_tools::blackInCheck(live)) && !legalMoves(live,sideToMove).empty(),"quiet-move/promotion/en passant fixture has a legal move");
        prepareSearch(sideToMove,false,0);useWinboard=false;
        check(petir_search::quiescenceSearch(sideToMove,320,321,live,0,2,0,321,petir_search::cutNode)==321 && nodeCount==0,"a legal quiet move or en passant prevents a false stalemate draw");
    }
    auto castling=fromFen("r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1",sideToMove);
    enPassantMove=parseMove("e1g1",castling,sideToMove);check(enPassantMove!=0,"short castling parsed");makeMove(castling,sideToMove,enPassantMove);
    check(castling.board[6]==6 && castling.board[5]==4 && isConsistent(castling),"castling updates rook, king, and hash");
    // Put a legal castle first with a shallow hash move, then force a root
    // beta cutoff. Both castle codes exceed the board's last square (63).
    for(int color=1;color<=2;++color)for(bool queenside:{false,true}) {
        newGame();
        auto castleRoot=fromFen(std::string("r3k2r/8/8/8/8/8/8/R3K2R ")+(color==1?"w":"b")+" KQkq - 0 1",sideToMove);
        const auto text=color==1?(queenside?"e1c1":"e1g1"):(queenside?"e8c8":"e8g8");
        const int castleMove=parseMove(text,castleRoot,color);
        prepareSearch(color,false,0);useWinboard=false;
        petir_hashing::storeHash(castleRoot.hashKey,color,0,castleMove,0,lowerBound,castleRoot.fiftyMoveCounter);
        const int score=petir_root::searchRoot(8,color,maxNode,-19001,-19000,castleRoot,false,0,0);
        const auto& ordering=color==1?whiteHistory:blackHistory;
        const int code=(castleMove>>7)&127;
        check(castleMove!=0 && rootBestMove==castleMove && score>=-19000,"castling root cutoff returns the legal castle "+std::string(text));
        check(ordering[color==1?6:-6][code]==1 && firstKillers[0]==castleMove,"castling root cutoff updates king history and killers "+std::string(text));
        check(isConsistent(castleRoot),"castling root search preserves its input board "+std::string(text));
    }
    for(char piece:std::string("qnbr")) {
        auto promotion=fromFen("7k/P7/8/8/8/8/8/7K w - - 0 1",sideToMove);
        enPassantMove=parseMove("a7a8"+std::string(1,piece),promotion,sideToMove);
        check(enPassantMove!=0,"accept opponent promotion "+std::string(1,piece));makeMove(promotion,sideToMove,enPassantMove);
        if(piece=='b') {
            check(promotion.whiteBishopCount==1,"bishop promotion increments the bishop count");
        }
        check(isConsistent(promotion),"promotion material and bitboards "+std::string(1,piece));
    }
    struct BishopPromotionFixture {const char* name;const char* fen;const char* promotion;const char* capture;};
    for(const auto& fixture:{
        BishopPromotionFixture{"white first bishop",
            "1r5k/P7/8/8/8/8/8/7K w - - 0 1","a7a8b","b8a8"},
        BishopPromotionFixture{"black first bishop",
            "7k/8/8/8/8/8/p7/1R5K b - - 0 1","a2a1b","b1a1"},
        BishopPromotionFixture{"white second bishop by capture",
            "1rr4k/P7/8/8/8/8/8/K4B2 w - - 0 1","a7b8b","c8b8"},
        BishopPromotionFixture{"black second bishop by capture",
            "k4b2/8/8/8/8/8/p7/1RR4K b - - 0 1","a2b1b","c1b1"}}) {
        int side=1;auto position=fromFen(fixture.fen,side);
        const int before=side==1?position.whiteBishopCount:position.blackBishopCount;
        const int promotionMove=parseMove(fixture.promotion,position,side);
        check(promotionMove!=0,std::string("bishop promotion is legal: ")+fixture.name);
        if(!promotionMove)continue;
        makeMove(position,side,promotionMove);
        check((side==1?position.whiteBishopCount:position.blackBishopCount)==before+1,
            std::string("bishop promotion increases existing count: ")+fixture.name);
        check(isConsistent(position),std::string("bishop promotion preserves incremental state: ")+fixture.name);
        const int captureMove=parseMove(fixture.capture,position,3-side);
        check(captureMove!=0,std::string("promoted bishop can be captured: ")+fixture.name);
        if(!captureMove)continue;
        makeMove(position,3-side,captureMove);
        check((side==1?position.whiteBishopCount:position.blackBishopCount)==before,
            std::string("capturing promoted bishop restores original count: ")+fixture.name);
        check(isConsistent(position),std::string("promoted bishop capture preserves incremental state: ")+fixture.name);
    }
    // Replay deterministic legal sequences to check all incremental representations.
    Position walk=start;sideToMove=1;std::uint32_t sequence=12345;
    for(int itemIndex=0;itemIndex<180;++itemIndex) {
        auto moves=legalMoves(walk,sideToMove);if(moves.empty()) {walk=start;sideToMove=1;continue;}
        sequence=sequence*1664525u+1013904223u;makeMove(walk,sideToMove,moves[sequence%moves.size()]);sideToMove=3-sideToMove;
        check(isConsistent(walk),"incremental state after random move "+std::to_string(itemIndex));
        (void)petir_ueval::evaluate(walk,sideToMove,-20500,20500);
    }
    // Verify unchanged packed binary books load and can answer an initial move.
    // Audit check evasions against full move generation followed by making
    // each move and checking the king, without relying on the evasion filter.
    int evasionPositions=0,noncheckPositions=0;
    prepareSearch(1,false,0);useWinboard=false;
    for(int game=0;game<32;++game) {
        Position audit=start;int auditSide=1;
        for(int plyIndex=0;plyIndex<160;++plyIndex) {
            auto auditMoves=legalMoves(audit,auditSide);
            const bool checked=auditSide==1?petir_tools::whiteInCheck(audit):petir_tools::blackInCheck(audit);
            if(checked) {
                ++evasionPositions;
                tmovelist candidates{};int count=0,checkCount=0;
                std::vector<int> reference,rawEvasions;
                auto collect=[&] {
                    for(int index=1;index<=count;++index) {
                        int destination=(candidates[index].move>>7)&127;
                        if(destination==shortCastleTarget || destination==longCastleTarget)continue;
                        auto child=audit;makeMove(child,auditSide,candidates[index].move);
                        if(!(auditSide==1?petir_tools::whiteInCheck(child):petir_tools::blackInCheck(child)))reference.push_back(candidates[index].move);
                    }
                };
                if(auditSide==1)petir_movgen::generateWhiteCaptures(audit,candidates,count);else petir_movgen::generateBlackCaptures(audit,candidates,count);
                collect();
                if(auditSide==1)petir_movgen::generateWhiteQuietMoves(audit,candidates,count);else petir_movgen::generateBlackQuietMoves(audit,candidates,count);
                collect();
                if(auditSide==1)petir_movgen::generateWhiteEvasions(audit,candidates,count,checkCount);else petir_movgen::generateBlackEvasions(audit,candidates,count,checkCount);
                for(int itemIndex=1;itemIndex<=count;++itemIndex)rawEvasions.push_back(candidates[itemIndex].move);
                std::sort(reference.begin(),reference.end());std::sort(rawEvasions.begin(),rawEvasions.end());
                if(reference!=rawEvasions) {
                    std::cerr<<"Evasion mismatch: "<<petir_tools::getFen(audit,auditSide).text<<'\n';
                    for(int move:reference)std::cerr<<" reference "<<moveText(move,audit,auditSide);
                    for(int move:rawEvasions)std::cerr<<" evasion "<<moveText(move,audit,auditSide);
                    std::cerr<<'\n';return 1;
                }
            } else {
                ++noncheckPositions;
                const auto nodes=nodeCount;
                const int score=petir_search::quiescenceSearch(auditSide,320,321,audit,0,2,0,321,petir_search::cutNode);
                if(score!=(auditMoves.empty()?drawScore:321) || nodeCount!=nodes) {
                    std::cerr<<"Stalemate probe mismatch: "<<toFen(audit,auditSide,1)<<'\n';return 1;
                }
            }
            if(auditMoves.empty())break;
            sequence=sequence*1664525u+1013904223u;
            makeMove(audit,auditSide,auditMoves[sequence%auditMoves.size()]);auditSide=3-auditSide;
        }
    }
    check(evasionPositions>0,"checked positions exercised by independent evasion audit");
    check(noncheckPositions>0,"quiescence stalemate probes match independently filtered legal moves");
    std::cout<<evasionPositions<<" checked positions audited for legal evasions\n";
    std::cout<<noncheckPositions<<" nonchecked positions audited for stalemate probes\n";
    if(std::filesystem::exists(pas::nativePath(pas::mainForm.defaultDirectory+"/white.book"))) {
        enPassantMove=petir_opening_book::findBookMove(start,1);check(enPassantMove!=0,"original white.book is compatible");
        auto moves=legalMoves(start,1);check(std::find(moves.begin(),moves.end(),enPassantMove)!=moves.end(),"book move is legal");
    }
    petir_repetition::clearRepetitionTable();
    check(petir_repetition::addRepetition(start.hashKey,1)==0,"first repetition insertion");
    check(petir_repetition::addRepetition(start.hashKey,1)==1,"second repetition insertion");
    petir_repetition::removeRepetition(start.hashKey,1);
    check(petir_repetition::addRepetition(start.hashKey,1)==1,"repetition delete and reinsert");
    // Keep previous results available; new searches age rather than clear them.
    petir_hashing::storeHash(start.hashKey,1,48,parseMove("e2e4",start,1),777,exactBound,start.fiftyMoveCounter);
    const auto cachedStart=[&] {
        int alpha=-20500,beta=20500,score=0,move=0;bool avoidNull=false;
        return petir_hashing::probeHash(start.hashKey,alpha,beta,8,1,score,move,avoidNull,start.fiftyMoveCounter);
    };
    check(cachedStart(),"previous search result is available before preparing a new search");
    whiteHistory[1][28]=blackHistory[-1][36]=1048576;
    firstKillers[0]=secondKillers[63]=1;
    prepareSearch(1,false,0);useWinboard=false;
    check(cachedStart(),"new search retains the previous search's hash result");
    const auto hashSlot=start.hashKey&static_cast<std::uint64_t>(hashTableSize-2); // the bucket's first entry
    check(petir_hashing_header::isOld(petir_hashing_header::hashTable[hashSlot]),"previous search's hash result is aged for replacement");
    const auto agedRecord=petir_hashing_header::hashTable[hashSlot];
    const auto collidingKey=start.hashKey^static_cast<std::uint64_t>(hashTableSize);
    petir_hashing::storeHash(collidingKey,1,8,parseMove("d2d4",start,1),321,exactBound,start.fiftyMoveCounter);
    check(petir_hashing_header::hashTable[hashSlot].key==agedRecord.key && petir_hashing_header::hashTable[hashSlot+1].key==collidingKey && !petir_hashing_header::isOld(petir_hashing_header::hashTable[hashSlot+1]),"a shallow entry leaves the previous search's deeper entry and is stamped current");
    petir_hashing_header::advanceHashGeneration();
    petir_hashing::storeHash(collidingKey,1,8,parseMove("d2d4",start,1),321,exactBound,start.fiftyMoveCounter);
    check(petir_hashing_header::hashTable[hashSlot].key==collidingKey && !petir_hashing_header::isOld(petir_hashing_header::hashTable[hashSlot]),"a shallow entry replaces a deeper one two searches old");
    petir_hashing_header::hashTable[hashSlot]=agedRecord;
    const auto flagsBeforeAge=agedRecord.flags;
    petir_hashing_header::advanceHashGeneration();
    check(petir_hashing_header::hashTable[hashSlot].flags==flagsBeforeAge && cachedStart(),"generation aging leaves cached entries and their bytes untouched");
    const auto generationBeforeWrap=petir_hashing_header::generation;
    for(int cycle=0;cycle<64;++cycle)petir_hashing_header::advanceHashGeneration();
    check(petir_hashing_header::generation==generationBeforeWrap && petir_hashing_header::hashTable[hashSlot].flags==flagsBeforeAge && cachedStart(),"generation wrap preserves hash contents and score validity");
    check(whiteHistory[1][28]==4096 && blackHistory[-1][36]==4096 && firstKillers[0]==0 && secondKillers[63]==0,"new search decays move-ordering history and resets killers");
    check(petir_repetition::addRepetition(start.hashKey,1)==2,"search reset preserves game repetition history");
    petir_repetition::clearRepetitionTable();
    petir_hashing_header::clearHashTable();
    check(!cachedStart(),"explicit Clear Hash discards the retained result");
    petir_hashing::storeHash(start.hashKey,1,48,parseMove("e2e4",start,1),777,exactBound,start.fiftyMoveCounter);
    whiteHistory[1][28]=blackHistory[-1][36]=1024;
    continuationHistory[1][28][1][36]=1024;
    petir_search::historyTried[1][28]=petir_search::historySucceeded[1][28]=1024;
    firstKillers[0]=secondKillers[63]=1;
    petir_repetition::addRepetition(start.hashKey,1);
    clearSearchState();prepareSearch(1,false,0);useWinboard=false;
    check(!cachedStart(),"fresh analysis discards previous hash results");
    check(whiteHistory[1][28]==0 && blackHistory[-1][36]==0 && continuationHistory[1][28][1][36]==0 && petir_search::historyTried[1][28]==0 && petir_search::historySucceeded[1][28]==0 && firstKillers[0]==0 && secondKillers[63]==0,"fresh analysis clears all move-ordering history and killers");
    check(petir_repetition::addRepetition(start.hashKey,1)==1,"fresh analysis preserves actual game repetition history");
    petir_repetition::clearRepetitionTable();
    // A deep exact root entry must guide ordering without replacing depth 1.
    const int rootHint=parseMove("e2e4",start,1);
    petir_hashing::storeHash(start.hashKey,1,48,rootHint,777,exactBound,start.fiftyMoveCounter);
    std::string firstRootMove;
    useWinboard=true;
    pas::output=[&](const pas::String& output) {
        if(!firstRootMove.empty())return;
        std::istringstream line(output.text);int depth=0,score=0,time=0,nodes=0;
        line>>depth>>score>>time>>nodes>>firstRootMove;
    };
    (void)petir_uiterative::iterativeDeepening(1,1,maxNode,start);
    pas::output={};useWinboard=false;
    check(nodeCount>0 && petir_uiterative::completedDepth==1 && petir_uiterative::completedScore!=777,"depth 1 searches root moves instead of accepting a cached root score");
    check(firstRootMove=="e2e4","root search tries the retained hash move first");
    petir_hashing_header::clearHashTable();
    (void)petir_uiterative::iterativeDeepening(5,1,maxNode,start);
    auto moves=legalMoves(start,1);
    check(std::find(moves.begin(),moves.end(),rootBestMove)!=moves.end(),"iterative search returns a legal starting move");
    check(isConsistent(start),"search does not change the caller's board");
    for(int requested=1;requested<=4;++requested) {
        newGame();prepareSearch(1,false,0);useWinboard=false;
        (void)petir_uiterative::iterativeDeepening(requested,1,maxNode,start);
        check(petir_uiterative::completedDepth==requested,"honor shallow depth limit "+std::to_string(requested));
    }
    newGame();
    const int restrictedMove=parseMove("a2a3",start,1);
    prepareSearch(1,false,0);useWinboard=false;
    petir::searchLimits.restrictRootMoves=true;petir::searchLimits.rootMoves={restrictedMove};
    (void)petir_uiterative::iterativeDeepening(3,1,maxNode,start);
    check(rootBestMove==restrictedMove && petir_uiterative::completedDepth==3,"searchmoves restricts the root through every iteration");
    check(!cachedStart(),"restricted root scores do not enter the unrestricted hash table");
    auto restrictedChild=start;makeMove(restrictedChild,1,restrictedMove);
    int childAlpha=-20500,childBeta=20500,childScore=0,childMove=0;bool childAvoidNull=false;
    check(petir_hashing::probeHash(restrictedChild.hashKey,childAlpha,childBeta,8,2,childScore,childMove,childAvoidNull,restrictedChild.fiftyMoveCounter),"restricted root searches retain ordinary child hash entries");
    prepareSearch(1,false,0);
    check(!petir::searchLimits.restrictRootMoves && petir::searchLimits.nodes<0 && petir::searchLimits.mate==0,"search preparation resets all UCI search limits");
    petir::searchLimits.nodes=std::int64_t(INT32_MAX)+5;nodeCount=INT32_MAX;
    auto countedChild=start;makeMove(countedChild,1,restrictedMove);
    check(nodeCount==std::int64_t(INT32_MAX)+1 && !petir::searchShouldStop(),"node counting and budgets continue past the 32-bit boundary");
    nodeCount=petir::searchLimits.nodes;
    check(petir::searchShouldStop() && !stopSearch.load(),"node exhaustion stops computation without releasing a held protocol result");
    newGame();prepareSearch(1,false,0);useWinboard=true;
    int previousBest=0;
    pas::output=[&](const pas::String& output) {
        std::istringstream line(output.text);int reported=0;line>>reported;
        if(reported==6 && !previousBest) {previousBest=petir_uiterative::completedMove;stopSearch=true;}
    };
    (void)petir_uiterative::iterativeDeepening(8,1,maxNode,start);
    pas::output={};useWinboard=false;
    check(previousBest!=0 && rootBestMove==previousBest && petir_uiterative::completedDepth==5,"interrupted iteration restores the last completed best move");
    // A quiet rook move up the board permits a back-rank mate; search must
    // choose a defense for either color, rather than just a legal move.
    for(const char* fen:{
        "4r1k1/5ppp/8/8/8/8/5PPP/R5K1 w - - 0 1",
        "r5k1/5ppp/8/8/8/8/5PPP/4R1K1 b - - 0 1",
        "7k/1q5p/5r1P/4Np2/R2p1P2/2bP2PK/1p6/1Q6 w - - 4 68"}) {
        newGame();
        auto threatened=fromFen(fen,sideToMove);
        prepareSearch(sideToMove,false,0);useWinboard=false;
        (void)petir_uiterative::iterativeDeepening(6,sideToMove,maxNode,threatened);
        auto defenses=legalMoves(threatened,sideToMove);
        const bool valid=std::find(defenses.begin(),defenses.end(),rootBestMove)!=defenses.end();
        check(valid,"mate-threat defense is a legal move");
        bool permitsMate=false;
        if(valid) {
            auto defended=threatened;makeMove(defended,sideToMove,rootBestMove);
            for(int reply:legalMoves(defended,3-sideToMove)) {
                auto afterReply=defended;makeMove(afterReply,3-sideToMove,reply);
                const bool checked=sideToMove==1?petir_tools::whiteInCheck(afterReply):petir_tools::blackInCheck(afterReply);
                if(checked && legalMoves(afterReply,sideToMove).empty())permitsMate=true;
            }
        }
        check(valid && !permitsMate,"search avoids an immediate checkmate threat");
    }
    // PetirCPP round 1 ended with 68.Qa2 Rxh6#. Checkmate must use
    // header's centipawn king value, not usee/movgen's private value 99.
    newGame();
    auto gameMate=fromFen("7k/1q5p/7r/4Np2/R2p1P2/2bP2PK/Qp6/8 w - - 0 69",sideToMove);
    check(petir_tools::whiteInCheck(gameMate) && legalMoves(gameMate,sideToMove).empty(),"PGN final position is checkmate");
    prepareSearch(sideToMove,false,0);useWinboard=false;
    check(petir_search::quiescenceSearch(sideToMove,-20500,20500,gameMate,1,2,0,petir_search::noValue,petir_search::pvNode)==-19998,"quiescence checkmate score uses centipawns");
    check(petir_search::mainSearch(16,sideToMove,-20500,20500,gameMate,false,2,0,false,0,petir_search::pvNode)==-19998,"main search checkmate score uses centipawns");
    check(petir_root::searchRoot(16,sideToMove,maxNode,-20500,20500,gameMate,false,0,0)<-19000,"root checkmate score uses centipawns");
    // Learning must update a packed 16-bit weight even in an odd probe slot.
    petir_opening_book::clearBook();
    for(int itemIndex=0;itemIndex<2;++itemIndex) {
        petir_opening_book::addBookPosition(2,whiteWinResult,1,2700,2700);
        petir_opening_book::addBookPosition(2+petir_opening_book::bookTableSize,whiteWinResult,1,2700,2700);
    }
    int weight=0;
    check(petir_opening_book::probeBook(2+petir_opening_book::bookTableSize,1,weight) && weight==4,"packed book learning and collision probing");
    std::cout<<checks<<" checks, "<<failures<<" failures\n";
    return failures?1:0;
}
}
