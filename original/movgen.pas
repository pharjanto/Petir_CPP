unit movgen;

interface
uses header;

procedure black_movgen_noncaps(var data:tdata;var movelist:tmovelist;var jml:integer);
procedure black_movgen_caps(var data:tdata;var movelist:tmovelist;var jml:integer);
procedure black_movgen_caps2(var data:tdata;var movelist:tmovelist;var jml:integer);
procedure q_black_movgen(var data:tdata;var movelist:tmovelist;var jml:integer);
procedure q_white_movgen(var data:tdata;var movelist:tmovelist;var jml:integer);
procedure white_movgen_caps(var data:tdata;var movelist:tmovelist;var jml:integer);
procedure white_movgen_caps2(var data:tdata;var movelist:tmovelist;var jml:integer);
procedure white_movgen_noncaps(var data:tdata;var movelist:tmovelist;var jml:integer;depth:integer);
procedure white_evasion(var data:tdata;var movelist:tmovelist;var jml,skakcount:integer);
procedure black_evasion(var data:tdata;var movelist:tmovelist;var jml,skakcount:integer);

implementation
uses bitboard_mask,tools,usee,makemove;

const
  _NILAI_PION=1;
  _NILAI_GAJAH=3;
  _NILAI_KUDA=3;
  _NILAI_BENTENG=5;
  _NILAI_MENTRI=9;
  _NILAI_RAJA=99;
  nilai_piece:array[-6..6] of byte=(_nilai_raja,_nilai_mentri,_nilai_benteng,_nilai_gajah,_nilai_kuda,_nilai_pion,0,_nilai_pion,_nilai_kuda,_nilai_gajah,_nilai_benteng,_nilai_mentri,_nilai_raja);


procedure white_evasion(var data:tdata;var movelist:tmovelist;var jml,skakcount:integer);
var piecepos,attack,battack,attack2:int64;
pos,post,posraja,dn,a:integer;
skak64:int64;
skakpos:integer;
path:int64;
allp,allp90,allpa8,allph8:int64;
begin
  jml:=0;skakcount:=0;
  battack:=0;
  posraja:=firstbitp(@data.kingwhite);
  skakpos:=64;

  piecepos:=data.knightblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    battack:=battack or knightmask[pos];
    if knightmask[pos] and data.kingwhite<>0 then
    begin
      inc(skakcount);skakpos:=pos;
    end;
  END;

  piecepos:=data.pawnblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    attack:=b_pawn_attack[pos];
    battack:=battack or attack;
    if attack and data.kingwhite<>0 then
    begin
      inc(skakcount);
      skakpos:=pos;
    end;
  end;

  allp:=data.allpieces and not data.kingwhite;
  allp90:=data.allpiecesr90 and not bit2nmask90[posraja];
  allpa8:=data.allpiecesa8h1 and not bit2nmaska8h1[posraja];
  allph8:=data.allpiecesh8a1 and not bit2nmaskh8a1[posraja];

  piecepos:=data.bishopblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    attack:=diaga8h1mask[pos,(Allpa8 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(allph8 shr h8a1shiftmask[pos]) and (255)];
    battack:=battack or attack;
    if attack and data.kingwhite<>0 then
    begin
      inc(skakcount);
      skakpos:=pos;
    end;
  end;

  piecepos:=data.rookblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    attack:=horzmask2[pos,(allp shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(allp90 shr vershiftmask[pos]) and 63];
    battack:=battack or attack;
    if attack and data.kingwhite<>0 then
    begin
      inc(skakcount);
      skakpos:=pos;
    end;
  end;

  piecepos:=data.queenblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];

    attack:=horzmask2[pos,(allp shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(allP90 shr vershiftmask[pos]) and 63];
    attack:=attack or diaga8h1mask[pos,(allpa8 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(allph8 shr h8a1shiftmask[pos]) and (255)];
    battack:=battack or attack;
    if attack and data.kingwhite<>0 then
    begin
      inc(skakcount);
      skakpos:=pos;
    end;
  end;

  pos:=lastbitp(@data.kingblack);
  battack:=battack or kingmask[pos];

  piecepos:=(kingmask[posraja] and not battack) and not data.whitepieces;
  while piecepos<>0 do
  begin
      post:=lastbitp(@piecepos);
      piecepos:=piecepos and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=posraja+ post shl 7;
      if data.papan[post]=0 then
        movelist[jml].score:=historyw[_RAJAPUTIH,post]
      else
        movelist[jml].score:=nilai_piece[data.papan[post]] shl 14;
  end;

  if skakcount=1 then
  begin
    if (data.ep<>_NO_EP) and (data.papan[skakpos]=_PIONHITAM) then
    begin
      if (data.papan[skakpos-1]=_PIONPUTIH) and (skakpos and 7 >0) then
      begin
        inc(jml);
        movelist[jml].moves:=(skakpos-1) + (_EN_PASSANT shl 7);
        movelist[jml].score:=0;
      end;
      if (data.papan[skakpos+1]=_PIONPUTIH) and (skakpos and 7 <7) then
      begin
        inc(jml);
        movelist[jml].moves:=(skakpos+1) + (_EN_PASSANT shl 7);
        movelist[jml].score:=0;
      end;
    end;
    skak64:=bit2nmask[skakpos];
    path:=pathmask[posraja,skakpos];
    piecepos:=data.knightwhite;
    dn:=direction[posraja,skakpos];
    while piecepos<>0 do
    BEGIN
      pos:=firstbitp(@piecepos);
      piecepos:=piecepos and bit2nmasknot[pos];
      attack:=knightmask[pos];
      attack2:=attack and skak64;
      if attack2 <>0 then
      begin
        post:=firstbitp(@attack2);
        if not white_open_check(data,pos+post shl 7) then
        begin
          inc(jml);
          movelist[jml].moves:=pos+ post shl 7;
          movelist[jml].score:=(nilai_piece[data.papan[post]]-_NILAI_KUDA) shl 15;
          if movelist[jml].score<0 then
            movelist[jml].score:=see(_SISIPUTIH,data,pos,post)shl 15;
        end;
      end;
      attack2:=attack and path;
      while attack2 <>0 do
      begin
        post:=firstbitp(@attack2);
        attack2:=attack2 and bit2nmasknot[post];
        if not white_open_check(data,pos+post shl 7) then
        begin
          inc(jml);
          movelist[jml].moves:=pos+ post shl 7;
          movelist[jml].score:=historyw[_KUDAPUTIH,post];
        end;
      end;
    END;

    piecepos:=data.pawnwhite;
    while piecepos<>0 do
    BEGIN
      pos:=firstbitp(@piecepos);
      piecepos:=piecepos and bit2nmasknot[pos];
      attack:=w_pawn_attack[pos];
      attack2:=attack and skak64;
      if attack2 <>0 then
      begin
        post:=firstbitp(@attack2);
        if not white_open_check(data,pos+post shl 7) then
        begin
          if pos shr 3 = 6 then
          begin
             inc(jml);
             movelist[jml].moves:=pos + (post) shl 7+ PROMOSI_MENTRI shl 14;
             movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_PION;
             inc(jml);
             movelist[jml].moves:=pos + (post) shl 7+ PROMOSI_KUDA shl 14;
             movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_PION;

          end else
          begin
            inc(jml);
            movelist[jml].moves:=pos+ post shl 7;
            movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_PION;
          end;
        end;
      end;
      attack2:=0;
      if data.papan[pos+8]=0 then
      begin
        attack2:=attack2 or bit2nmask[pos+8];
        if (pos shr 3 = 1) and (data.papan[pos+16]=0) then
          attack2:=attack2 or bit2nmask[pos+16];
      end;
      attack2:=attack2 and path;
      if attack2 <>0 then
      begin
        post:=firstbitp(@attack2);
        if not white_open_check(data,pos+post shl 7) then
        begin
          if pos shr 3 = 6 then
          begin
            for a:=PROMOSI_MENTRI to PROMOSI_KUDA do
            begin
             inc(jml);
             movelist[jml].moves:=pos + (post) shl 7+ a shl 14;
             movelist[jml].score:=see(_SISIPUTIH,data,pos,post)shl 15;
             if movelist[jml].score=0 then movelist[jml].score:=3shl 15;
            end;
          end else
          begin
            inc(jml);
            movelist[jml].moves:=pos+ post shl 7;
            movelist[jml].score:=historyw[_PIONPUTIH,post];
          end;
        end;
      end;
    end;

    piecepos:=data.bishopwhite;
    while piecepos<>0 do
    BEGIN

      pos:=firstbitp(@piecepos);
      piecepos:=piecepos and bit2nmasknot[pos];
      attack:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
      attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
      attack2:=attack and skak64;
      if attack2 <>0 then
      begin
          post:=firstbitp(@attack2);
          if not white_open_check(data,pos+post shl 7) then
          begin
            inc(jml);
            movelist[jml].moves:=pos+ post shl 7;
            movelist[jml].score:=(nilai_piece[data.papan[post]]-_NILAI_GAJAH) shl 15;
            if movelist[jml].score<0 then
              movelist[jml].score:=see(_SISIPUTIH,data,pos,post)shl 15;
          end;
      end;
      attack2:=attack and path;
      while attack2 <>0 do
      begin
          post:=firstbitp(@attack2);
          attack2:=attack2 and bit2nmasknot[post];
          if not white_open_check(data,pos+post shl 7) then
          begin
            inc(jml);
            movelist[jml].moves:=pos+ post shl 7;
            movelist[jml].score:=historyw[_GAJAHPUTIH,post];
          end;
      end;
    end;

    piecepos:=data.rookwhite;
    while piecepos<>0 do
    BEGIN
      pos:=firstbitp(@piecepos);
      piecepos:=piecepos and bit2nmasknot[pos];
      attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
      attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
      attack2:=attack and skak64;
      if attack2 <>0 then
      begin
          post:=firstbitp(@attack2);
          if not white_open_check(data,pos+post shl 7) then
          begin
            inc(jml);
            movelist[jml].moves:=pos+ post shl 7;
            movelist[jml].score:=(nilai_piece[data.papan[post]]-_NILAI_BENTENG) shl 15;
            if movelist[jml].score<0 then
              movelist[jml].score:=see(_SISIPUTIH,data,pos,post)shl 15;
          end;
      end;
      attack2:=attack and path;
      while attack2 <>0 do
      begin
          post:=firstbitp(@attack2);
          attack2:=attack2 and bit2nmasknot[post];
          if not white_open_check(data,pos+post shl 7) then
          begin
            inc(jml);
            movelist[jml].moves:=pos+ post shl 7;
            movelist[jml].score:=historyw[_BENTENGPUTIH,post];
          end;
      end;
    end;

    piecepos:=data.queenwhite;
    while piecepos<>0 do
    BEGIN
      pos:=firstbitp(@piecepos);
      piecepos:=piecepos and bit2nmasknot[pos];
      attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
      attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
      attack:=attack or diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
      attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
      attack2:=attack and skak64;
      if attack2 <>0 then
      begin
          post:=firstbitp(@attack2);
          if not white_open_check(data,pos+post shl 7) then
          begin
            inc(jml);
            movelist[jml].moves:=pos+ post shl 7;
            movelist[jml].score:=(nilai_piece[data.papan[post]]-_NILAI_MENTRI) shl 15;
            if movelist[jml].score<0 then
              movelist[jml].score:=see(_SISIPUTIH,data,pos,post) shl 15;
          end;
      end;
      attack2:=attack and path;
      while attack2 <>0 do
      begin
          post:=firstbitp(@attack2);
          attack2:=attack2 and bit2nmasknot[post];
          if not white_open_check(data,pos+post shl 7) then
          begin
            inc(jml);
            movelist[jml].moves:=pos+ post shl 7;
            movelist[jml].score:=historyw[_MENTRIPUTIH,post];
          end;
      end;
    end;

  end;

end;

procedure black_evasion(var data:tdata;var movelist:tmovelist;var jml,skakcount:integer);
var piecepos,attack,battack,attack2:int64;
pos,post,posraja:integer;
skak64:int64;
skakpos,a:integer;
path:int64;
allp,allp90,allpa8,allph8:int64;
begin
  jml:=0;skakcount:=0;
  battack:=0;
  posraja:=lastbitp(@data.kingblack);
  skakpos:=64;

  piecepos:=data.knightwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    battack:=battack or knightmask[pos];
    if knightmask[pos] and data.kingblack<>0 then
    begin
      inc(skakcount);skakpos:=pos;
    end;
  END;

  piecepos:=data.pawnwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    attack:=w_pawn_attack[pos];
    battack:=battack or attack;
    if attack and data.kingblack<>0 then
    begin
      inc(skakcount);
      skakpos:=pos;
    end;
  end;

  allp:=data.allpieces and not data.kingblack;
  allp90:=data.allpiecesr90 and not bit2nmask90[posraja];
  allpa8:=data.allpiecesa8h1 and not bit2nmaska8h1[posraja];
  allph8:=data.allpiecesh8a1 and not bit2nmaskh8a1[posraja];

  piecepos:=data.bishopwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    attack:=diaga8h1mask[pos,(allpa8 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(allph8 shr h8a1shiftmask[pos]) and (255)];
    battack:=battack or attack;
    if attack and data.kingblack<>0 then
    begin
      inc(skakcount);
      skakpos:=pos;
    end;
  end;

  piecepos:=data.rookwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    attack:=horzmask2[pos,(allp shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(allp90 shr vershiftmask[pos]) and 63];
    battack:=battack or attack;
    if attack and data.kingblack<>0 then
    begin
      inc(skakcount);
      skakpos:=pos;
    end;
  end;

  piecepos:=data.queenwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];

    attack:=horzmask2[pos,(allp shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(allP90 shr vershiftmask[pos]) and 63];
    attack:=attack or diaga8h1mask[pos,(allpa8 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(allph8 shr h8a1shiftmask[pos]) and (255)];
    battack:=battack or attack;
    if attack and data.kingblack<>0 then
    begin
      inc(skakcount);
      skakpos:=pos;
    end;
  end;

  pos:=lastbitp(@data.kingwhite);
  battack:=battack or kingmask[pos];

  piecepos:=(kingmask[posraja] and not battack) and not data.blackpieces;
  while piecepos<>0 do
  begin
      post:=firstbitp(@piecepos);
      piecepos:=piecepos and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=posraja+ post shl 7;
      if data.papan[post]=0 then
        movelist[jml].score:=historyb[_RAJAHITAM,post]
      else
        movelist[jml].score:=nilai_piece[data.papan[post]]shl 15;
  end;

  if skakcount=1 then
  begin
    if (data.ep<>_NO_EP) AND (data.papan[skakpos]=_PIONPUTIH)then
    begin
      if (data.papan[skakpos-1]=_PIONHITAM) and (skakpos and 7 >0) then
      begin
        inc(jml);
        movelist[jml].moves:=(skakpos-1) + (_EN_PASSANT shl 7);
        movelist[jml].score:=0;
      end;
      if (data.papan[skakpos+1]=_PIONHITAM) and (skakpos and 7 <7) then
      begin
        inc(jml);
        movelist[jml].moves:=(skakpos+1) + (_EN_PASSANT shl 7);
        movelist[jml].score:=0;
      end;
    end;

    skak64:=bit2nmask[skakpos];
    path:=pathmask[posraja,skakpos];
    piecepos:=data.knightblack;
    while piecepos<>0 do
    BEGIN
      pos:=lastbitp(@piecepos);
      piecepos:=piecepos and bit2nmasknot[pos];
      attack:=knightmask[pos];
      attack2:=attack and skak64;
      if attack2 <>0 then
      begin
        post:=lastbitp(@attack2);
        if not black_open_check(data,pos+post shl 7) then
        begin
          inc(jml);
          movelist[jml].moves:=pos+ post shl 7;
          movelist[jml].score:=(nilai_piece[data.papan[post]]-_NILAI_KUDA) shl 15;
          if movelist[jml].score<0 then
             movelist[jml].score:=see(_SISIHITAM,data,pos,post)shl 15;
        end;
      end;
      attack2:=attack and path;
      while attack2 <>0 do
      begin
        post:=lastbitp(@attack2);
        attack2:=attack2 and bit2nmasknot[post];
        if not black_open_check(data,pos+post shl 7) then
        begin
          inc(jml);
          movelist[jml].moves:=pos+ post shl 7;
          movelist[jml].score:=historyb[_KUDAHITAM,post];
        end;
      end;
    END;

    piecepos:=data.pawnblack;
    while piecepos<>0 do
    BEGIN
      pos:=lastbitp(@piecepos);
      piecepos:=piecepos and bit2nmasknot[pos];
      attack:=b_pawn_attack[pos];
      attack2:=attack and skak64;
      if attack2 <>0 then
      begin
          post:=lastbitp(@attack2);
          if not black_open_check(data,pos+post shl 7) then
          begin
            if pos shr 3=1 then
            begin
              for a:=PROMOSI_MENTRI to PROMOSI_KUDA do
              begin
                inc(jml);
                movelist[jml].moves:=pos + (post) shl 7 + a shl 14;
                movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_PION;
              end;
            end else
            begin
              inc(jml);
              movelist[jml].moves:=pos+ post shl 7;
              movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_PION;
            end;
          end;
      end;
      attack2:=0;
      if data.papan[pos-8]=0 then
      begin
          attack2:=attack2 or bit2nmask[pos-8];
          if (pos shr 3 = 6) and (data.papan[pos-16]=0) then
            attack2:=attack2 or bit2nmask[pos-16];
      end;
      attack2:=attack2 and path;
      if attack2 <>0 then
      begin
          post:=lastbitp(@attack2);
          if not black_open_check(data,pos+post shl 7) then
          begin
            if pos shr 3=1 then
            begin
              for a:=PROMOSI_MENTRI to PROMOSI_KUDA do
              begin
                inc(jml);
                movelist[jml].moves:=pos + (post) shl 7 + a shl 14;
                movelist[jml].score:=see(_SISIHITAM,data,pos,post) shl 15;
                if movelist[jml].score=0 then movelist[jml].score:=3shl 15;
              end;
            end else
            begin
              inc(jml);
              movelist[jml].moves:=pos+ post shl 7;
              movelist[jml].score:=historyb[_PIONHITAM,post];
            end;
          end;
      end;
    end;

    piecepos:=data.bishopblack;
    while piecepos<>0 do
    BEGIN
      pos:=lastbitp(@piecepos);
      piecepos:=piecepos and bit2nmasknot[pos];
      attack:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
      attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
      attack2:=attack and skak64;
      if attack2 <>0 then
      begin
          post:=lastbitp(@attack2);
          if not black_open_check(data,pos+post shl 7) then
          begin
            inc(jml);
            movelist[jml].moves:=pos+ post shl 7;
            movelist[jml].score:=(nilai_piece[data.papan[post]]-_NILAI_GAJAH) shl 15;
            if movelist[jml].score<0 then
              movelist[jml].score:=see(_SISIHITAM,data,pos,post)shl 15;
          end;
      end;
      attack2:=attack and path;
      while attack2 <>0 do
      begin
          post:=lastbitp(@attack2);
          attack2:=attack2 and bit2nmasknot[post];
          if not black_open_check(data,pos+post shl 7) then
          begin
            inc(jml);
            movelist[jml].moves:=pos+ post shl 7;
            movelist[jml].score:=historyb[_GAJAHHITAM,post];
          end;
      end;
    end;
    piecepos:=data.rookblack;
    while piecepos<>0 do
    BEGIN
      pos:=lastbitp(@piecepos);
      piecepos:=piecepos and bit2nmasknot[pos];
      attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
      attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
      attack2:=attack and skak64;
      if attack2 <>0 then
      begin
        post:=lastbitp(@attack2);
        if not black_open_check(data,pos+post shl 7) then
        begin
          inc(jml);
          movelist[jml].moves:=pos+ post shl 7;
          movelist[jml].score:=(nilai_piece[data.papan[post]]-_NILAI_BENTENG)shl 15;
          if movelist[jml].score<0 then
            movelist[jml].score:=see(_SISIHITAM,data,pos,post)shl 15;
        end;
      end;
      attack2:=attack and path;
      while attack2 <>0 do
      begin
        post:=lastbitp(@attack2);
        attack2:=attack2 and bit2nmasknot[post];
        if not black_open_check(data,pos+post shl 7) then
        begin
          inc(jml);
          movelist[jml].moves:=pos+ post shl 7;
          movelist[jml].score:=historyb[_BENTENGHITAM,post];
        end;
      end;
    end;

    piecepos:=data.queenblack;
    while piecepos<>0 do
    BEGIN
      pos:=lastbitp(@piecepos);
      piecepos:=piecepos and bit2nmasknot[pos];
      attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
      attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
      attack:=attack or diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
      attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
      attack2:=attack and skak64;
      if attack2 <>0 then
      begin
        post:=lastbitp(@attack2);
        if not black_open_check(data,pos+post shl 7) then
        begin
          inc(jml);
          movelist[jml].moves:=pos+ post shl 7;
          movelist[jml].score:=(nilai_piece[data.papan[post]]-_NILAI_MENTRI)shl 15;
          if movelist[jml].score<0 then
            movelist[jml].score:=see(_SISIHITAM,data,pos,post) shl 15;
        end;
      end;
      attack2:=attack and path;
      while attack2 <>0 do
      begin
        post:=lastbitp(@attack2);
        attack2:=attack2 and bit2nmasknot[post];
        if not black_open_check(data,pos+post shl 7) then
        begin
          inc(jml);
          movelist[jml].moves:=pos+ post shl 7;
          movelist[jml].score:=historyb[_MENTRIHITAM,post];
        end;
      end;
    end;
  end;
end;

procedure black_movgen_noncaps;
var piecepos,attack:int64;
pos,post:integer;
p:^Integer;
begin
  jml:=0;
  p:=@movelist;
  dec(p);

  piecepos:=data.pawnblack and not rank_mask[1];
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    IF (data.papan[pos-8]=0) {and (pos>=a3)} THEN
    BEGIN
      inc(jml);
{      movelist[jml].moves:=pos+(pos - 8) shl 7;
      movelist[jml].score:=historyb[pos+(pos - 8) shl 7];}
      inc(p);
      p^:=pos+(pos - 8) shl 7;
      inc(p);
      p^:=historyb[_PIONHITAM,pos-8];


      IF (pos>=a7) AND (data.papan[pos-16]=0) THEN
      BEGIN
        inc(jml);
        inc(p);
        p^:=pos+(pos - 16) shl 7;
        inc(p);
        p^:=historyb[_PIONHITAM,pos-16];

{        movelist[jml].moves:=pos + (pos - 16) shl 7;
        movelist[jml].score:=historyb[pos + (pos - 16) shl 7]}
      END;
    END;
  END;

  piecepos:=data.knightblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    attack:=knightmask[pos] and not data.allpieces;
    WHILE attack<>0 DO
    BEGIN
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      inc(p);
      p^:=pos+ post shl 7;
      inc(p);
      p^:=historyb[_KUDAHITAM,post];

{      movelist[jml].moves:=pos+ post shl 7;
      movelist[jml].score:=historyb[pos+ post shl 7]}
    END;
  END;

  piecepos:=data.queenblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];

    attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
    attack:=attack or diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    attack:=attack and not data.allpieces;

    while attack<>0 do
    begin
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(p);
      p^:=pos+ post shl 7;
      inc(p);
      p^:=historyb[_MENTRIHITAM,post];
      inc(jml);
{      movelist[jml].moves:=pos+ post shl 7;
      movelist[jml].score:=historyb[pos+ post shl 7]}
    end;
  END;

  piecepos:=data.bishopblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    attack:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    attack:=attack and not data.allpieces;

     while attack<>0 do
     begin
       post:=lastbitp(@attack);
       attack:=attack and bit2nmasknot[post];
       inc(jml);
       inc(p);
       p^:=pos+ post shl 7;
       inc(p);
       p^:=historyb[_GAJAHHITAM,post];

{       movelist[jml].moves:=pos+ post shl 7;
       movelist[jml].score:=historyb[pos+ post shl 7]}
     end;
  END;

  piecepos:=data.rookblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];

    attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
    attack:=attack and not data.allpieces;
    while attack<>0 do
    begin
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      inc(p);
      p^:=pos+ post shl 7;
      inc(p);
      p^:=historyb[_BENTENGHITAM,post];

{      movelist[jml].moves:=pos + post shl 7;

      movelist[jml].score:=historyb[pos+ post shl 7]}
    end;
  END;

  pos:=lastbitp(@data.kingblack);
  attack:=kingmask[pos] and not data.allpieces;
  WHILE attack<>0 DO
  BEGIN
    post:=lastbitp(@attack);
    attack:=attack and bit2nmasknot[post];
    inc(jml);
    movelist[jml].moves:=pos+post shl 7;

    movelist[jml].score:=historyb[_RAJAHITAM,post]
  END;

  if (data.flagrokade and BITHITAMNOC<>0) and
  (data.papan[e8]=_RAJAHITAM) and  
  not black_attacked(data,e8)
  THEN
  BEGIN

    IF (data.papan[f8]=0) AND (data.papan[g8]=0)
    and (data.papan[h8]=_BENTENGHITAM)
    AND NOT black_attacked(data,f8)
    AND NOT black_attacked(data,g8)
    and (data.flagrokade AND BITHITAMSC<>0)
    THEN
    BEGIN
      inc(jml);
      movelist[jml].moves:=pos + _ROKADEPENDEK shl 7;
      movelist[jml].score:=historyb[_RAJAHITAM, _ROKADEPENDEK];
    END;

    IF (data.papan[d8]=0) AND (data.papan[c8]=0) AND (data.papan[b8]=0)
    and (data.flagrokade AND BITHITAMLC<>0)
    AND (data.papan[a8]=_BENTENGHITAM)    
    AND NOT black_attacked(data,d8)
    AND NOT black_attacked(data,c8)

    THEN
    BEGIN
      inc(jml);
      movelist[jml].moves:=pos+ _ROKADEPANJANG shl 7;
      movelist[jml].score:=historyb[_RAJAHITAM, _ROKADEPANJANG];
    END;
  END;

end;

procedure black_movgen_caps;
var piecepos,attack:int64;
pos,post,a:integer;
p:^Integer;
begin
  jml:=0;
  p:=@movelist;
  piecepos:=data.pawnblack and rank_mask[1];
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if (pos and 7>0) and (data.papan[pos-9]>0) then
    begin
         inc(jml);
         movelist[jml].moves:=pos + (pos-9) shl 7+ PROMOSI_MENTRI shl 14;
         movelist[jml].score:=(nilai_piece[data.papan[pos-9]]-_NILAI_PION)+5;
         inc(jml);
         movelist[jml].moves:=pos + (pos-9) shl 7+ PROMOSI_KUDA shl 14;
         movelist[jml].score:=(nilai_piece[data.papan[pos-9]]-_NILAI_PION)+5;

    end;
    if (pos and 7<7) and (data.papan[pos-7]>0) then
    begin
         inc(jml);
         movelist[jml].moves:=pos + (pos-7) shl 7+ PROMOSI_MENTRI shl 14;
         movelist[jml].score:=(nilai_piece[data.papan[pos-7]]-_NILAI_PION)+5;
         inc(jml);
         movelist[jml].moves:=pos + (pos-7) shl 7+ PROMOSI_KUDA shl 14;
         movelist[jml].score:=(nilai_piece[data.papan[pos-7]]-_NILAI_PION)+5;

    end;
    if (data.papan[pos-8]=0) then
    begin
       inc(jml);
       movelist[jml].moves:=pos + (pos-8) shl 7+ PROMOSI_MENTRI shl 14;
       movelist[jml].score:=see(_SISIPUTIH,data,pos,pos-8);
       if movelist[jml].score=0 then movelist[jml].score:=3;
       inc(jml);
       movelist[jml].moves:=pos + (pos-8) shl 7+ PROMOSI_KUDA shl 14;
       movelist[jml].score:=see(_SISIPUTIH,data,pos,pos-8);
       if movelist[jml].score=0 then movelist[jml].score:=3;

    end;
  end;

  piecepos:=data.pawnblack and not rank_mask[1];
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    a:=pos and 7;
    if(data.papan[pos-9]>0) and (a >0)then
    begin
        inc(jml);
        movelist[jml].moves:=pos + (pos-9) shl 7;
        movelist[jml].score:=nilai_piece[data.papan[pos-9]]-_NILAI_PION;
    end;
    if (data.papan[pos-7]>0)and (a <7) then
    begin
        inc(jml);
        movelist[jml].moves:=pos+ (pos-7) shl 7;
        movelist[jml].score:=nilai_piece[data.papan[pos-7]]-_NILAI_PION;
    end;
    IF (pos shr 3=3) and
    (abs((pos and 7)-data.ep)=1)
    THEN
    BEGIN
      inc(jml);
      movelist[jml].moves:=pos+ _en_passant shl 7;
      movelist[jml].score:=_NILAI_PION;
    END;
  END;

  piecepos:=data.knightblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    attack:=knightmask[pos] and data.whitepieces;
    WHILE attack<>0 DO
    BEGIN
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_KUDA;
      if movelist[jml].score<=0 then
        movelist[jml].score:=see(_SISIHITAM,data,pos,post);
    END;
  END;

  piecepos:=data.bishopblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    attack:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    attack:=attack and data.whitepieces;

     while attack<>0 do
     begin
       post:=lastbitp(@attack);
       attack:=attack and bit2nmasknot[post];
       inc(jml);
       movelist[jml].moves:=pos + (post shl 7);
       movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_GAJAH;
      if movelist[jml].score<=0 then
        movelist[jml].score:=see(_SISIHITAM,data,pos,post);

     end;
  END;

  piecepos:=data.rookblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];

    attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
    attack:=attack and (data.whitepieces);
    while attack<>0 do
    begin
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_BENTENG;
      if movelist[jml].score<=0 then
        movelist[jml].score:=see(_SISIHITAM,data,pos,post);

    end;
  END;

  piecepos:=data.queenblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];

    attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
    attack:=attack or diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    attack:=attack and data.whitepieces;

    while attack<>0 do
    begin
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=see(_SISIHITAM,data,pos,post);

    end;
  END;

  pos:=lastbitp(@data.kingblack);
  attack:=kingmask[pos] and data.whitepieces;
  WHILE attack<>0 DO
  BEGIN
    post:=lastbitp(@attack);
    attack:=attack and bit2nmasknot[post];
    inc(jml);
    movelist[jml].moves:=pos + (post shl 7);
    movelist[jml].score:=see(_SISIHITAM,data,pos,post);

  END;

end;

procedure black_movgen_caps2;
var piecepos,attack:int64;
pos,post,a:integer;
begin
  jml:=0;
  piecepos:=data.pawnblack and rank_mask[1];
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if (pos and 7>0) and (data.papan[pos-9]>0) then
    begin
         inc(jml);
         movelist[jml].moves:=pos + (pos-9) shl 7+ PROMOSI_MENTRI shl 14;
         movelist[jml].score:=(nilai_piece[data.papan[pos-9]]-_NILAI_PION)+5;
         inc(jml);
         movelist[jml].moves:=pos + (pos-9) shl 7+ PROMOSI_KUDA shl 14;
         movelist[jml].score:=(nilai_piece[data.papan[pos-9]]-_NILAI_PION)+5;

    end;
    if (pos and 7<7) and (data.papan[pos-7]>0) then
    begin
         inc(jml);
         movelist[jml].moves:=pos + (pos-7) shl 7+ PROMOSI_MENTRI shl 14;
         movelist[jml].score:=(nilai_piece[data.papan[pos-7]]-_NILAI_PION)+5;
         inc(jml);
         movelist[jml].moves:=pos + (pos-7) shl 7+ PROMOSI_KUDA shl 14;
         movelist[jml].score:=(nilai_piece[data.papan[pos-7]]-_NILAI_PION)+5;

    end;
    if (data.papan[pos-8]=0) then
    begin
       inc(jml);
       movelist[jml].moves:=pos + (pos-8) shl 7+ PROMOSI_MENTRI shl 14;
       movelist[jml].score:=see(_SISIPUTIH,data,pos,pos-8);
       if movelist[jml].score=0 then movelist[jml].score:=3;
       inc(jml);
       movelist[jml].moves:=pos + (pos-8) shl 7+ PROMOSI_KUDA shl 14;
       movelist[jml].score:=see(_SISIPUTIH,data,pos,pos-8);
       if movelist[jml].score=0 then movelist[jml].score:=3;

    end;
  end;
  piecepos:=data.pawnblack and (rank_mask[2] or rank_mask[3]);
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if (data.papan[pos-8]=0) then
    begin
         inc(jml);
         movelist[jml].moves:=pos+ (pos-8) shl 7 ;
         movelist[jml].score:=see(_SISIHITAM,data,pos,pos-8);
         if movelist[jml].score=0 then movelist[jml].score:=1;
    end;
  end;

  piecepos:=data.pawnblack and not rank_mask[1];
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    a:=pos and 7;
    if(data.papan[pos-9]>0) and (a >0)then
    begin
        inc(jml);
        movelist[jml].moves:=pos + (pos-9) shl 7;
        movelist[jml].score:=nilai_piece[data.papan[pos-9]]-_NILAI_PION;
    end;
    if (data.papan[pos-7]>0)and (a <7) then
    begin
        inc(jml);
        movelist[jml].moves:=pos+ (pos-7) shl 7;
        movelist[jml].score:=nilai_piece[data.papan[pos-7]]-_NILAI_PION;
    end;
    IF (pos shr 3=3) and
    (abs((pos and 7)-data.ep)=1)
    THEN
    BEGIN
      inc(jml);
      movelist[jml].moves:=pos+ _en_passant shl 7;
      movelist[jml].score:=_NILAI_PION;
    END;
  END;

  piecepos:=data.knightblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    attack:=knightmask[pos] and data.whitepieces;
    WHILE attack<>0 DO
    BEGIN
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_KUDA;
      if movelist[jml].score<=0 then
        movelist[jml].score:=see(_SISIHITAM,data,pos,post);
    END;
  END;

  piecepos:=data.bishopblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    attack:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    attack:=attack and data.whitepieces;

     while attack<>0 do
     begin
       post:=lastbitp(@attack);
       attack:=attack and bit2nmasknot[post];
       inc(jml);
       movelist[jml].moves:=pos + (post shl 7);
       movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_GAJAH;
      if movelist[jml].score<=0 then
        movelist[jml].score:=see(_SISIHITAM,data,pos,post);

     end;
  END;

  piecepos:=data.rookblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];

    attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
    attack:=attack and (data.whitepieces);
    while attack<>0 do
    begin
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_BENTENG;
      if movelist[jml].score<=0 then
        movelist[jml].score:=see(_SISIHITAM,data,pos,post);

    end;
  END;

  piecepos:=data.queenblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];

    attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
    attack:=attack or diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    attack:=attack and data.whitepieces;

    while attack<>0 do
    begin
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=see(_SISIHITAM,data,pos,post);

    end;
  END;

  pos:=lastbitp(@data.kingblack);
  attack:=kingmask[pos] and data.whitepieces;
  WHILE attack<>0 DO
  BEGIN
    post:=lastbitp(@attack);
    attack:=attack and bit2nmasknot[post];
    inc(jml);
    movelist[jml].moves:=pos + (post shl 7);
    movelist[jml].score:=see(_SISIHITAM,data,pos,post);

  END;
  attack:=kingmask[pos] and (data.pawnwhite shl 8); //and not data.blackpieces and not data.whitepieces;
  while attack<>0 do
  begin
    post:=firstbitp(@attack);
    attack:=attack and bit2nmasknot[post];
    if (data.papan[post]=0) and (w_pion_bebas_mask[post-8] and data.pawnblack = 0) then
    begin
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=0;
    end;
  end;

end;


procedure Q_black_movgen;
var piecepos,attack,t64:int64;
pos,post,a,posraja:integer;
attackb,attackr,attackn:int64;
begin
  jml:=0;
  posraja:=firstbitp(@data.kingwhite);
//  attackb:=attackbishop(data,posraja);
  attackb:=diaga8h1mask[posraja,(data.allpiecesa8h1 shr a8h1shiftmask[posraja]) and (255)];
  attackb:=attackb or diagh8a1mask[posraja,(data.allpiecesh8a1 shr h8a1shiftmask[posraja]) and (255)];

  attackr:=horzmask2[posraja,(data.allpieces shr (posraja and 56+1)) and 63];
  attackr:=attackr or vertmask2[posraja,(data.allpiecesr90 shr vershiftmask[posraja]) and 63];

  attackn:=knightmask[posraja];

  piecepos:=((data.pawnblack and mask_left) shr 17) and data.kingwhite;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    inc(pos,17);
    if data.papan[pos-8]=0 then
    begin
      inc(jml);
      movelist[jml].moves:=pos + (pos-8) shl 7;
      movelist[jml].score:=see(_SISIHITAM,data,pos,pos-8);
    end;
  end;

  piecepos:=((data.pawnblack and mask_right) shr 15) and data.kingwhite;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    inc(pos,15);
    if data.papan[pos-8]=0 then
    begin
      inc(jml);
      movelist[jml].moves:=pos + (pos-8) shl 7;
      movelist[jml].score:=see(_SISIHITAM,data,pos,pos-8);
    end;
  end;


  piecepos:=((data.pawnblack and mask_left) shr 9) and data.whitepieces;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    pos:=pos+9;
    if pos shr 3=1 then
    begin
        inc(jml);
        movelist[jml].moves:=pos + (pos-9) shl 7 + PROMOSI_MENTRI shl 14;
        movelist[jml].score:=(nilai_piece[data.papan[pos-9]]-_NILAI_PION)+3;
    end else
    begin
      inc(jml);
      movelist[jml].moves:=pos + (pos-9) shl 7;
      movelist[jml].score:=nilai_piece[data.papan[pos-9]]-_NILAI_PION;
    end;
  end;

  piecepos:=((data.pawnblack and mask_right) shr 7) and data.whitepieces;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    pos:=pos+7;
    if pos shr 3=1 then
    begin
        inc(jml);
        movelist[jml].moves:=pos+ (pos-7) shl 7 + PROMOSI_MENTRI shl 14;
        movelist[jml].score:=(nilai_piece[data.papan[pos-7]]-_NILAI_PION)+3;
    end else
    begin
      inc(jml);
      movelist[jml].moves:=pos+ (pos-7) shl 7;
      movelist[jml].score:=nilai_piece[data.papan[pos-7]]-_NILAI_PION;
    end;
  end;
  piecepos:=data.pawnblack and rank_mask[1];
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if (data.papan[pos-8]=0) then
    begin
         inc(jml);
         movelist[jml].moves:=pos+ (pos-8) shl 7 + PROMOSI_MENTRI shl 14;
         movelist[jml].score:=see(_SISIHITAM,data,pos,pos-8);
         if movelist[jml].score=0 then movelist[jml].score:=3;
    end;
  end;
  piecepos:=data.pawnblack and (rank_mask[2] or rank_mask[3]);
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if (data.papan[pos-8]=0) then
    begin
         inc(jml);
         movelist[jml].moves:=pos+ (pos-8) shl 7 ;
         movelist[jml].score:=see(_SISIHITAM,data,pos,pos-8);
         if movelist[jml].score=0 then movelist[jml].score:=1;
    end;
  end;

  if data.ep<>_NO_EP then
  begin
    piecepos:=data.pawnblack and rank_mask[3];
    while piecepos<>0 do
    begin
      pos:=firstbitp(@piecepos);
      piecepos:=piecepos and bit2nmasknot[pos];
      if
       (abs((pos and 7)-data.ep)=1)
      THEN
      BEGIN
        inc(jml);
        movelist[jml].moves:=pos + (_EN_PASSANT shl 7);
        movelist[jml].score:=_NILAI_PION;
      END;

    end;
  end;


  piecepos:=data.knightblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    attack:=knightmask[pos] and data.whitepieces;
    WHILE attack<>0 DO
    BEGIN
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_KUDA;
      if movelist[jml].score<0 then
        movelist[jml].score:=see(_SISIHITAM,data,pos,post);
    END;
    attack:=(knightmask[pos] and attackn) and not data.allpieces;
    while attack<>0 do
    begin
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=see(_SISIHITAM,data,pos,post);
    end;
  END;

  piecepos:=data.bishopblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    attack:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    t64:=attack;
    attack:=attack and data.whitepieces;

     while attack<>0 do
     begin
       post:=lastbitp(@attack);
       attack:=attack and bit2nmasknot[post];
       inc(jml);
       movelist[jml].moves:=pos + (post shl 7);
       movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_GAJAH;
       if movelist[jml].score<0 then
         movelist[jml].score:=see(_SISIHITAM,data,pos,post);
     end;
     attack:=(t64 and attackb) and not data.allpieces;
     while attack<>0 do
     begin
         post:=lastbitp(@attack);
         attack:=attack and bit2nmasknot[post];

         inc(jml);
         movelist[jml].moves:=pos + (post shl 7);
         movelist[jml].score:=see(_SISIHITAM,data,pos,post);
     end;
  END;

  piecepos:=data.rookblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];

    attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
    t64:=attack;
    attack:=attack and data.whitepieces;
    while attack<>0 do
    begin
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_BENTENG;
      if movelist[jml].score<0 then
        movelist[jml].score:=see(_SISIHITAM,data,pos,post);
    end;
    attack:=(t64 and attackr) and not data.allpieces;
    while attack <>0 do
    begin
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=see(_SISIHITAM,data,pos,post);

    end;
  END;

  piecepos:=data.queenblack;
  while piecepos<>0 do
  BEGIN
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];

    attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
    attack:=attack or diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    t64:=attack;
    attack:=attack and data.whitepieces;

    while attack<>0 do
    begin
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=see(_SISIHITAM,data,pos,post);
    end;
    attack:=(t64 and (attackr or attackb)) and not data.allpieces;
    while attack<>0 do
    begin
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=see(_SISIHITAM,data,pos,post);
    end;

  END;

  pos:=lastbitp(@data.kingblack);
  attack:=kingmask[pos] and data.whitepieces;
  WHILE attack<>0 DO
  BEGIN
    post:=lastbitp(@attack);
    attack:=attack and bit2nmasknot[post];
    inc(jml);
    movelist[jml].moves:=pos + (post shl 7);
    movelist[jml].score:=see(_SISIHITAM,data,pos,post);

  END;

  attack:=kingmask[pos] and (data.pawnwhite shl 8); //and not data.blackpieces and not data.whitepieces;
  while attack<>0 do
  begin
    post:=firstbitp(@attack);
    attack:=attack and bit2nmasknot[post];
    if (data.papan[post]=0) and (w_pion_bebas_mask[post-8] and data.pawnblack = 0) then
    begin

      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=0;
    end;
  end;

end;


procedure white_movgen_noncaps;
var piecepos,attack:int64;
pos,post:integer;
begin
  jml:=0;

  piecepos:=data.pawnwhite and not rank_mask[6];
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
//    clear(@piecepos,pos);
    IF {(pos<=h6) and }(data.papan[pos+8]=0) THEN
    BEGIN
      inc(jml);
      movelist[jml].moves:=pos + ((pos+8) shl 7);
      movelist[jml].score:=historyw[_PIONPUTIH,pos+8];
      IF (pos<=h2) AND (data.papan[pos+16]=0) THEN
      BEGIN
        inc(jml);
        movelist[jml].moves:=pos + (pos+16) shl 7;
        movelist[jml].score:=historyw[_PIONPUTIH,pos+16];
      END;
    END;
  END;

  piecepos:=data.knightwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
//    clear(@piecepos,pos);
    attack:=knightmask[pos] and not data.allpieces;
    WHILE attack<>0 DO
    BEGIN
      post:=firstbitp(@attack);
      attack:=attack and bit2nmasknot[post];
//      clear(@attack,post);
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=historyw[_KUDAPUTIH,post];
      if (depth<=3) and (See(_SISIPUTIH,data,pos,post)<0) then movelist[jml].score:=0;
    END;
  END;

  piecepos:=data.queenwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    //clear(@piecepos,pos);

    attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
    attack:=attack or diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    attack:=attack and not data.allpieces;

    while attack<>0 do
    begin
      post:=firstbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      //clear(@attack,post);
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=historyw[_MENTRIPUTIH,post];
      if (depth<=3) and (See(_SISIPUTIH,data,pos,post)<0) then movelist[jml].score:=0;
    end;
  END;

  piecepos:=data.bishopwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    //clear(@piecepos,pos);
    attack:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    attack:=attack and not data.allpieces;

     while attack<>0 do
     begin
       post:=firstbitp(@attack);
       attack:=attack and bit2nmasknot[post];
       inc(jml);
       movelist[jml].moves:=pos + (post shl 7);
       movelist[jml].score:=historyw[_GAJAHPUTIH,post];
       if (depth<=3) and (See(_SISIPUTIH,data,pos,post)<0) then movelist[jml].score:=0;
     end;
  END;

  piecepos:=data.rookwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
//    clear(@piecepos,pos);

    attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
    attack:=attack and not data.allpieces;
    while attack<>0 do
    begin
      post:=firstbitp(@attack);
      attack:=attack and bit2nmasknot[post];
//      clear(@attack,post);
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=historyw[_BENTENGPUTIH,post];
      if (depth<=3) and (See(_SISIPUTIH,data,pos,post)<0) then movelist[jml].score:=0;
    end;
  END;

  pos:=firstbitp(@data.kingwhite);
  attack:=kingmask[pos] and not data.allpieces;
  WHILE attack<>0 DO
  BEGIN
    post:=firstbitp(@attack);
    attack:=attack and bit2nmasknot[post];
    //clear(@attack,post);
    inc(jml);
    movelist[jml].moves:=pos + (post shl 7);
    movelist[jml].score:=historyw[_RAJAPUTIH,post]
  END;

  if (data.flagrokade and BITPUTIHNOC<>0) and
  (data.papan[e1]=_RAJAPUTIH) and
  NOT white_attacked(data,e1)
  THEN
  BEGIN

    IF (data.papan[f1]=0) AND (data.papan[g1]=0)
    AND (data.flagrokade AND BITPUTIHSC<>0)
    AND (data.papan[h1]=_BENTENGPUTIH)
    AND NOT white_attacked(data,f1)
    AND NOT white_attacked(data,g1)

    THEN
    BEGIN
      inc(jml);
      movelist[jml].moves:=pos + (_ROKADEPENDEK shl 7);
      movelist[jml].score:=historyw[_RAJAPUTIH,_ROKADEPENDEK];
    END;

    IF (data.papan[d1]=0) AND (data.papan[c1]=0) AND (data.papan[b1]=0)
    AND (data.flagrokade AND BITPUTIHLC<>0)
    AND (data.papan[a1]=_BENTENGPUTIH)    
    AND NOT white_attacked(data,d1)
    AND NOT white_attacked(data,c1)

    THEN
    BEGIN
      inc(jml);
      movelist[jml].moves:=pos + (_ROKADEPANJANG shl 7);
      movelist[jml].score:=historyw[_RAJAPUTIH,_ROKADEPANJANG ];
    END;
  END;

end;


procedure white_movgen_caps;
var piecepos,attack,temp:int64;
pos,post,a:integer;
begin
  jml:=0;
  piecepos:=data.pawnwhite and rank_mask[6];
  while piecepos<>0 do
  begin
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if (pos and 7<7) and (data.papan[pos+9]<0) then
    begin
         inc(jml);
         movelist[jml].moves:=pos + (pos+9) shl 7+ PROMOSI_MENTRI shl 14;
         movelist[jml].score:=(nilai_piece[data.papan[pos+9]]-_NILAI_PION)+5;
         inc(jml);
         movelist[jml].moves:=pos + (pos+9) shl 7+ PROMOSI_KUDA shl 14;
         movelist[jml].score:=(nilai_piece[data.papan[pos+9]]-_NILAI_PION)+5;

    end;
    if (pos and 7>0) and (data.papan[pos+7]<0)  then
    begin
         inc(jml);
         movelist[jml].moves:=pos + (pos+7) shl 7+ PROMOSI_MENTRI shl 14;
         movelist[jml].score:=(nilai_piece[data.papan[pos+7]]-_NILAI_PION)+5;
         inc(jml);
         movelist[jml].moves:=pos + (pos+7) shl 7+ PROMOSI_KUDA shl 14;
         movelist[jml].score:=(nilai_piece[data.papan[pos+7]]-_NILAI_PION)+5;

    end;
    if (data.papan[pos+8]=0) then
    begin
       inc(jml);
       movelist[jml].moves:=pos + (pos+8) shl 7+ PROMOSI_MENTRI shl 14;
       movelist[jml].score:=see(_SISIPUTIH,data,pos,pos+8);
       if movelist[jml].score=0 then movelist[jml].score:=3;
       inc(jml);
       movelist[jml].moves:=pos + (pos+8) shl 7+ PROMOSI_KUDA shl 14;
       movelist[jml].score:=see(_SISIPUTIH,data,pos,pos+8);
       if movelist[jml].score=0 then movelist[jml].score:=3;
    end;
  end;
  temp:=data.pawnwhite and not rank_mask[6];
  piecepos:=((temp and mask_left) shl 7) and data.blackpieces;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    pos:=pos-7;
    inc(jml);
    movelist[jml].moves:=pos + (pos+7) shl 7;
    movelist[jml].score:=nilai_piece[data.papan[pos+7]]-_NILAI_PION;
  end;

  piecepos:=((temp and mask_right) shl 9) and data.blackpieces;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    pos:=pos-9;
    inc(jml);
    movelist[jml].moves:=pos + (pos+9) shl 7;
    movelist[jml].score:=nilai_piece[data.papan[pos+9]]-_NILAI_PION;
  end;
  if data.ep<>_NO_EP then
  begin
    piecepos:=data.pawnwhite and rank_mask[4];
    while piecepos<>0 do
    begin
      pos:=firstbitp(@piecepos);
      piecepos:=piecepos and bit2nmasknot[pos];
      if
       (abs((pos and 7)-data.ep)=1)
      THEN
      BEGIN
        inc(jml);
        movelist[jml].moves:=pos + (_EN_PASSANT shl 7);
        movelist[jml].score:=_NILAI_PION;
      END;

    end;
  end;

  piecepos:=data.knightwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    attack:=knightmask[pos] and data.blackpieces;
    WHILE attack<>0 DO
    BEGIN
      post:=firstbitp(@attack);
      //anuu:=post;
//      attack:=attack and (attack -1);
      attack:=attack and not bit2nmask[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_KUDA;
      if movelist[jml].score<=0 then
        movelist[jml].score:=see(_SISIPUTIH,data,pos,post);

    END;
  END;

  piecepos:=data.bishopwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    attack:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    attack:=attack and data.blackpieces;

     while attack<>0 do
     begin
       post:=lastbitp(@attack);
       attack:=attack and bit2nmasknot[post];
       inc(jml);
       movelist[jml].moves:=pos + (post shl 7);
       movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_GAJAH;
      if movelist[jml].score<=0 then
        movelist[jml].score:=see(_SISIPUTIH,data,pos,post);

     end;
  END;

  piecepos:=data.rookwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];

    attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
    attack:=attack and (data.blackpieces);
    while attack<>0 do
    begin
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_BENTENG;
      if movelist[jml].score<=0 then
        movelist[jml].score:=see(_SISIPUTIH,data,pos,post);

    end;
  END;

  piecepos:=data.queenwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];

    attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
    attack:=attack or diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    attack:=attack and data.blackpieces;

    while attack<>0 do
    begin
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=see(_SISIPUTIH,data,pos,post);

    end;
  END;

  pos:=firstbitp(@data.kingwhite);
  attack:=kingmask[pos] and data.blackpieces;
  WHILE attack<>0 DO
  BEGIN
    post:=firstbitp(@attack);
    attack:=attack and bit2nmasknot[post];
    inc(jml);
    movelist[jml].moves:=pos + (post shl 7);
        movelist[jml].score:=see(_SISIPUTIH,data,pos,post);
  END;
end;

procedure white_movgen_caps2;
var piecepos,attack,temp:int64;
pos,post,a:integer;
begin
  jml:=0;
  piecepos:=data.pawnwhite and rank_mask[6];
  while piecepos<>0 do
  begin
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if (pos and 7<7) and (data.papan[pos+9]<0) then
    begin
         inc(jml);
         movelist[jml].moves:=pos + (pos+9) shl 7+ PROMOSI_MENTRI shl 14;
         movelist[jml].score:=(nilai_piece[data.papan[pos+9]]-_NILAI_PION)+5;
         inc(jml);
         movelist[jml].moves:=pos + (pos+9) shl 7+ PROMOSI_KUDA shl 14;
         movelist[jml].score:=(nilai_piece[data.papan[pos+9]]-_NILAI_PION)+5;

    end;
    if (pos and 7>0) and (data.papan[pos+7]<0)  then
    begin
         inc(jml);
         movelist[jml].moves:=pos + (pos+7) shl 7+ PROMOSI_MENTRI shl 14;
         movelist[jml].score:=(nilai_piece[data.papan[pos+7]]-_NILAI_PION)+5;
         inc(jml);
         movelist[jml].moves:=pos + (pos+7) shl 7+ PROMOSI_KUDA shl 14;
         movelist[jml].score:=(nilai_piece[data.papan[pos+7]]-_NILAI_PION)+5;

    end;
    if (data.papan[pos+8]=0) then
    begin
       inc(jml);
       movelist[jml].moves:=pos + (pos+8) shl 7+ PROMOSI_MENTRI shl 14;
       movelist[jml].score:=see(_SISIPUTIH,data,pos,pos+8);
       if movelist[jml].score=0 then movelist[jml].score:=3;
       inc(jml);
       movelist[jml].moves:=pos + (pos+8) shl 7+ PROMOSI_KUDA shl 14;
       movelist[jml].score:=see(_SISIPUTIH,data,pos,pos+8);
       if movelist[jml].score=0 then movelist[jml].score:=3;
    end;
  end;

  piecepos:=data.pawnwhite and (rank_mask[5] or rank_mask[4]);
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if (data.papan[pos+8]=0) then
    begin
       inc(jml);
       movelist[jml].moves:=pos + (pos+8) shl 7;
       movelist[jml].score:=see(_SISIPUTIH,data,pos,pos+8);
       if movelist[jml].score=0 then movelist[jml].score:=1;
    end;
  end;


  temp:=data.pawnwhite and not rank_mask[6];
  piecepos:=((temp and mask_left) shl 7) and data.blackpieces;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    pos:=pos-7;
    inc(jml);
    movelist[jml].moves:=pos + (pos+7) shl 7;
    movelist[jml].score:=nilai_piece[data.papan[pos+7]]-_NILAI_PION;
  end;

  piecepos:=((temp and mask_right) shl 9) and data.blackpieces;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    pos:=pos-9;
    inc(jml);
    movelist[jml].moves:=pos + (pos+9) shl 7;
    movelist[jml].score:=nilai_piece[data.papan[pos+9]]-_NILAI_PION;
  end;
  if data.ep<>_NO_EP then
  begin
    piecepos:=data.pawnwhite and rank_mask[4];
    while piecepos<>0 do
    begin
      pos:=firstbitp(@piecepos);
      piecepos:=piecepos and bit2nmasknot[pos];
      if
       (abs((pos and 7)-data.ep)=1)
      THEN
      BEGIN
        inc(jml);
        movelist[jml].moves:=pos + (_EN_PASSANT shl 7);
        movelist[jml].score:=_NILAI_PION;
      END;

    end;
  end;

  piecepos:=data.knightwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    attack:=knightmask[pos] and data.blackpieces;
    WHILE attack<>0 DO
    BEGIN
      post:=firstbitp(@attack);
      //anuu:=post;
//      attack:=attack and (attack -1);
      attack:=attack and not bit2nmask[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_KUDA;
      if movelist[jml].score<=0 then
        movelist[jml].score:=see(_SISIPUTIH,data,pos,post);

    END;
  END;

  piecepos:=data.bishopwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    attack:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    attack:=attack and data.blackpieces;

     while attack<>0 do
     begin
       post:=lastbitp(@attack);
       attack:=attack and bit2nmasknot[post];
       inc(jml);
       movelist[jml].moves:=pos + (post shl 7);
       movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_GAJAH;
      if movelist[jml].score<=0 then
        movelist[jml].score:=see(_SISIPUTIH,data,pos,post);

     end;
  END;

  piecepos:=data.rookwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];

    attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
    attack:=attack and (data.blackpieces);
    while attack<>0 do
    begin
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_BENTENG;
      if movelist[jml].score<=0 then
        movelist[jml].score:=see(_SISIPUTIH,data,pos,post);

    end;
  END;

  piecepos:=data.queenwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];

    attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
    attack:=attack or diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    attack:=attack and data.blackpieces;

    while attack<>0 do
    begin
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=see(_SISIPUTIH,data,pos,post);

    end;
  END;

  pos:=firstbitp(@data.kingwhite);
  attack:=kingmask[pos] and data.blackpieces;
  WHILE attack<>0 DO
  BEGIN
    post:=firstbitp(@attack);
    attack:=attack and bit2nmasknot[post];
    inc(jml);
    movelist[jml].moves:=pos + (post shl 7);
        movelist[jml].score:=see(_SISIPUTIH,data,pos,post);

  END;
  attack:=kingmask[pos] and (data.pawnblack shr 8); //and not data.blackpieces and not data.whitepieces;
  while attack<>0 do
  begin
    post:=firstbitp(@attack);
    attack:=attack and bit2nmasknot[post];
    if (data.papan[post]=0) and (b_pion_bebas_mask[post+8] and data.pawnwhite = 0) then
    begin

      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=0;
    end;
  end;


end;


procedure q_white_movgen;
var piecepos,attack,t64:int64;
pos,post,a,posraja:integer;
attackb,attackr,attackn,target:int64;
begin
  jml:=0;
  posraja:=lastbitp(@data.kingblack);
  attackb:=diaga8h1mask[posraja,(data.allpiecesa8h1 shr a8h1shiftmask[posraja]) and (255)];
  attackb:=attackb or diagh8a1mask[posraja,(data.allpiecesh8a1 shr h8a1shiftmask[posraja]) and (255)];

  attackr:=horzmask2[posraja,(data.allpieces shr (posraja and 56+1)) and 63];
  attackr:=attackr or vertmask2[posraja,(data.allpiecesr90 shr vershiftmask[posraja]) and 63];
  attackn:=knightmask[posraja];

  piecepos:=((data.pawnwhite and mask_left) shl 15) and data.kingblack;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    dec(pos,15);
    if data.papan[pos+8]=0 then
    begin
      inc(jml);
      movelist[jml].moves:=pos + (pos+8) shl 7;
      movelist[jml].score:=see(_SISIPUTIH,data,pos,pos+8);
    end;
  end;

  piecepos:=((data.pawnwhite and mask_right) shl 17) and data.kingblack;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    dec(pos,17);
    if data.papan[pos+8]=0 then
    begin
      inc(jml);
      movelist[jml].moves:=pos + (pos+8) shl 7;
      movelist[jml].score:=see(_SISIPUTIH,data,pos,pos+8);
    end;
  end;


  piecepos:=((data.pawnwhite and mask_left) shl 7) and data.blackpieces;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    pos:=pos-7;
    if pos shr 3 = 6 then
    begin
       inc(jml);
       movelist[jml].moves:=pos + (pos+7) shl 7+ PROMOSI_MENTRI shl 14;
       movelist[jml].score:=(nilai_piece[data.papan[pos+7]]-_NILAI_PION)+3;
    end else
    begin
      inc(jml);
      movelist[jml].moves:=pos + (pos+7) shl 7;
      movelist[jml].score:=nilai_piece[data.papan[pos+7]]-_NILAI_PION;
    end;
  end;

  piecepos:=((data.pawnwhite and mask_right) shl 9) and data.blackpieces;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    pos:=pos-9;
    if pos shr 3 = 6 then
    begin
       inc(jml);
       movelist[jml].moves:=pos + (pos+9) shl 7+ PROMOSI_MENTRI shl 14;
       movelist[jml].score:=(nilai_piece[data.papan[pos+9]]-_NILAI_PION)+3;

    end else
    begin
      inc(jml);
      movelist[jml].moves:=pos + (pos+9) shl 7;
      movelist[jml].score:=nilai_piece[data.papan[pos+9]]-_NILAI_PION;
    end;
  end;
  piecepos:=data.pawnwhite and rank_mask[6];
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if (data.papan[pos+8]=0) then
    begin
       inc(jml);
       movelist[jml].moves:=pos + (pos+8) shl 7+ PROMOSI_MENTRI shl 14;
       movelist[jml].score:=see(_SISIPUTIH,data,pos,pos+8);
       if movelist[jml].score=0 then movelist[jml].score:=3;
    end;
  end;
  piecepos:=data.pawnwhite and (rank_mask[5] or rank_mask[4]);
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if (data.papan[pos+8]=0) then
    begin
       inc(jml);
       movelist[jml].moves:=pos + (pos+8) shl 7;
       movelist[jml].score:=see(_SISIPUTIH,data,pos,pos+8);
       if movelist[jml].score=0 then movelist[jml].score:=1;
    end;
  end;

  if data.ep<>_NO_EP then
  begin
    piecepos:=data.pawnwhite and rank_mask[4];
    while piecepos<>0 do
    begin
      pos:=firstbitp(@piecepos);
      piecepos:=piecepos and bit2nmasknot[pos];
      if
       (abs((pos and 7)-data.ep)=1)
      THEN
      BEGIN
        inc(jml);
        movelist[jml].moves:=pos + (_EN_PASSANT shl 7);
        movelist[jml].score:=_NILAI_PION;
      END;

    end;
  end;

  piecepos:=data.knightwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];

    attack:=knightmask[pos] and data.blackpieces;
    WHILE attack<>0 DO
    BEGIN
      post:=firstbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_KUDA;
      if movelist[jml].score<0 then
        movelist[jml].score:=see(_SISIPUTIH,data,pos,post);
    END;
    attack:=(knightmask[pos] and attackn) and not data.allpieces;
    while attack<>0 do
    begin
      post:=firstbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=see(_SISIPUTIH,data,pos,post);
    end;
  END;

  piecepos:=data.bishopwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    attack:=diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    t64:=attack;
    attack:=attack and data.blackpieces;

     while attack<>0 do
     begin
       post:=lastbitp(@attack);
       attack:=attack and bit2nmasknot[post];
       inc(jml);
       movelist[jml].moves:=pos + (post shl 7);
       movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_GAJAH;
       if movelist[jml].score<0 then
         movelist[jml].score:=see(_SISIPUTIH,data,pos,post);
     end;

     attack:=(t64 and attackb) and not data.allpieces;
     while attack<>0 do
     begin
       post:=lastbitp(@attack);
       attack:=attack and bit2nmasknot[post];
       inc(jml);
       movelist[jml].moves:=pos + (post shl 7);
       movelist[jml].score:=see(_SISIPUTIH,data,pos,post);

     end;
  END;

  piecepos:=data.rookwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];

    attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
    t64:=attack;
    attack:=attack and data.blackpieces;
    while attack<>0 do
    begin
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=nilai_piece[data.papan[post]]-_NILAI_BENTENG;
      if movelist[jml].score<0 then
        movelist[jml].score:=see(_SISIPUTIH,data,pos,post);
    end;
    attack:=(t64 and attackr) and not data.allpieces;
    while attack<>0 do
    begin
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=see(_SISIPUTIH,data,pos,post);

    end;
  END;

  piecepos:=data.queenwhite;
  while piecepos<>0 do
  BEGIN
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];

    attack:=horzmask2[pos,(data.allpieces shr (pos and 56+1)) and 63];
    attack:=attack or vertmask2[pos,(data.allpiecesr90 shr vershiftmask[pos]) and 63];
    attack:=attack or diaga8h1mask[pos,(data.allpiecesa8h1 shr a8h1shiftmask[pos]) and (255)];
    attack:=attack or diagh8a1mask[pos,(data.allpiecesh8a1 shr h8a1shiftmask[pos]) and (255)];
    t64:=attack;
    attack:=attack and data.blackpieces;

    while attack<>0 do
    begin
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=see(_SISIPUTIH,data,pos,post);
    end;
    attack:=(t64 and (attackb or attackr)) and not data.allpieces;
    while attack<>0 do
    begin
      post:=lastbitp(@attack);
      attack:=attack and bit2nmasknot[post];
      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=see(_SISIPUTIH,data,pos,post);
    end;
  END;

  pos:=firstbitp(@data.kingwhite);
  attack:=kingmask[pos] and data.blackpieces;
  WHILE attack<>0 DO
  BEGIN
    post:=firstbitp(@attack);
    attack:=attack and bit2nmasknot[post];
    inc(jml);
    movelist[jml].moves:=pos + (post shl 7);
        movelist[jml].score:=see(_SISIPUTIH,data,pos,post);
  END;

  attack:=kingmask[pos] and (data.pawnblack shr 8);// and not data.blackpieces and not data.whitepieces;
  while attack<>0 do
  begin
    post:=firstbitp(@attack);
    attack:=attack and bit2nmasknot[post];
    if (data.papan[post]=0) and (b_pion_bebas_mask[post+8] and data.pawnwhite = 0) then
    begin

      inc(jml);
      movelist[jml].moves:=pos + (post shl 7);
      movelist[jml].score:=0;
    end;
  end;


end;



end.
