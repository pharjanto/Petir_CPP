unit Makemove;

interface
uses header;

procedure makewhitemove(moves:integer;var data:tdata);
procedure makeblackmove(moves:integer;var data:tdata);

implementation
uses bitboard_mask,hashing_header,unit1;

procedure cekerror(data:tdata);forward;

procedure whiterokadependek(VAR data:tdata);
BEGIN
  data.hashkey:=data.hashkey xor hashkey[_BENTENGPUTIH,h1];
  data.hashkey:=data.hashkey xor hashkey[_RAJAPUTIH,e1];
  data.hashkey:=data.hashkey xor hashkey[_BENTENGPUTIH,f1];
  data.hashkey:=data.hashkey xor hashkey[_RAJAPUTIH,g1];
  data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
  data.wc:=ROKADEPENDEK;

  data.flagrokade:=data.flagrokade and not BITPUTIHNOC;
//  data.flagrokade:=data.flagrokade and not BITPUTIHLC;
  data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
  data.papan[e1]:=0;
  data.papan[h1]:=0;
  data.papan[f1]:=_BENTENGPUTIH;
  data.papan[g1]:=_RAJAPUTIH;
//  inc(data.jumlah_langkah);


  data.kingwhite:=data.kingwhite and bit2nmasknot[e1];
  data.rookwhite:=data.rookwhite and bit2nmasknot[h1];
  data.rookqueen:=data.rookqueen and bit2nmasknot[h1];
  data.rookwhite:=data.rookwhite or bit2nmask[f1];
  data.rookqueen:=data.rookqueen or bit2nmask[f1];
  data.kingwhite:=data.kingwhite or bit2nmask[g1];


  data.whitepieces:=data.whitepieces and bit2nmasknot[e1];
  data.whitepieces:=data.whitepieces and bit2nmasknot[h1];
  data.whitepieces:=data.whitepieces or bit2nmask[f1];
  data.whitepieces:=data.whitepieces or bit2nmask[g1];

  data.allpiecesr90:=data.allpiecesr90 and not bit2nmask90[e1];
  data.allpiecesr90:=data.allpiecesr90 and not bit2nmask90[h1];
  data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[f1];
  data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[g1];

  data.allpiecesa8h1:=data.allpiecesa8h1 and not bit2nmaska8h1[e1];
  data.allpiecesa8h1:=data.allpiecesa8h1 and not bit2nmaska8h1[h1];
  data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[f1];
  data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[g1];

  data.allpiecesh8a1:=data.allpiecesh8a1 and not bit2nmaskh8a1[e1];
  data.allpiecesh8a1:=data.allpiecesh8a1 and not bit2nmaskh8a1[h1];
  data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[f1];
  data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[g1];


  data.allpieces:=data.blackpieces or data.whitepieces;
  data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
  data.ep:=_NO_EP;
  data.hashkey:=data.hashkey xor enpassanthashkey[_NO_EP];
END;

procedure whiterokadepanjang(VAR data:tdata);
BEGIN
  //mengupdate hash keu untuk rokade panjang
  data.hashkey:=data.hashkey xor hashkey[_BENTENGPUTIH,a1];
  data.hashkey:=data.hashkey xor hashkey[_RAJAPUTIH,e1];
  data.hashkey:=data.hashkey xor hashkey[_BENTENGPUTIH,d1];
  data.hashkey:=data.hashkey xor hashkey[_RAJAPUTIH,c1];
  data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];

  data.flagrokade:=data.flagrokade and not BITPUTIHNOC;
//  data.flagrokade:=data.flagrokade and not BITPUTIHLC;
  data.wc:=ROKADEPANJANG;

  data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];

  //mengupdate petak pada papan
  data.papan[e1]:=0;
  data.papan[a1]:=0;
  data.papan[d1]:=_BENTENGPUTIH;
  data.papan[c1]:=_RAJAPUTIH;
//  inc(data.jumlah_langkah);


  data.kingwhite:=data.kingwhite and bit2nmasknot[e1];
  data.rookwhite:=data.rookwhite and bit2nmasknot[a1];
  data.rookqueen:=data.rookqueen and bit2nmasknot[a1];
  data.rookwhite:=data.rookwhite or bit2nmask[d1];
  data.rookqueen:=data.rookqueen or bit2nmask[d1];
  data.kingwhite:=data.kingwhite or bit2nmask[c1];


  data.whitepieces:=data.whitepieces and bit2nmasknot[e1];
  data.whitepieces:=data.whitepieces and bit2nmasknot[a1];
  data.whitepieces:=data.whitepieces or bit2nmask[d1];
  data.whitepieces:=data.whitepieces or bit2nmask[c1];

  data.allpiecesr90:=data.allpiecesr90 and not bit2nmask90[e1];
  data.allpiecesr90:=data.allpiecesr90 and not bit2nmask90[a1];
  data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[d1];
  data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[c1];

  data.allpiecesa8h1:=data.allpiecesa8h1 and not bit2nmaska8h1[e1];
  data.allpiecesa8h1:=data.allpiecesa8h1 and not bit2nmaska8h1[a1];
  data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[d1];
  data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[c1];

  data.allpiecesh8a1:=data.allpiecesh8a1 and not bit2nmaskh8a1[e1];
  data.allpiecesh8a1:=data.allpiecesh8a1 and not bit2nmaskh8a1[a1];
  data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[d1];
  data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[c1];

  data.allpieces:=data.blackpieces or data.whitepieces;
  data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
  data.ep:=_NO_EP;
  data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
END;

procedure blackrokadependek(VAR data:tdata);
BEGIN
  data.hashkey:=data.hashkey xor hashkey[_BENTENGHITAM,h8];//hash key menggunakan zorbist key
  data.hashkey:=data.hashkey xor hashkey[_RAJAHITAM,e8];
  data.hashkey:=data.hashkey xor hashkey[_BENTENGHITAM,f8];
  data.hashkey:=data.hashkey xor hashkey[_RAJAHITAM,g8];
  data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];

  data.flagrokade:=data.flagrokade and not BITHITAMNOC;
  data.bc:=ROKADEPENDEK;

  data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
  data.papan[e8]:=0;
  data.papan[h8]:=0;
  data.papan[f8]:=_BENTENGHITAM;
  data.papan[g8]:=_RAJAHITAM;
//  inc(data.jumlah_langkah);


  data.blackpieces:=data.blackpieces and bit2nmasknot[e8];
  data.blackpieces:=data.blackpieces and bit2nmasknot[h8];
  data.blackpieces:=data.blackpieces or bit2nmask[f8];
  data.blackpieces:=data.blackpieces or bit2nmask[g8];

  data.kingblack:=data.kingblack and bit2nmasknot[e8];
  data.rookblack:=data.rookblack and bit2nmasknot[h8];
  data.rookqueen:=data.rookqueen and bit2nmasknot[h8];
  data.rookblack:=data.rookblack or bit2nmask[f8];
  data.rookqueen:=data.rookqueen or bit2nmask[f8];
  data.kingblack:=data.kingblack or bit2nmask[g8];

  data.allpiecesr90:=data.allpiecesr90 and not bit2nmask90[e8];
  data.allpiecesr90:=data.allpiecesr90 and not bit2nmask90[h8];
  data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[f8];
  data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[g8];

  data.allpiecesa8h1:=data.allpiecesa8h1 and not bit2nmaska8h1[e8];
  data.allpiecesa8h1:=data.allpiecesa8h1 and not bit2nmaska8h1[h8];
  data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[f8];
  data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[g8];

  data.allpiecesh8a1:=data.allpiecesh8a1 and not bit2nmaskh8a1[e8];
  data.allpiecesh8a1:=data.allpiecesh8a1 and not bit2nmaskh8a1[h8];
  data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[f8];
  data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[g8];

  data.allpieces:=data.blackpieces or data.whitepieces;
  data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
  data.ep:=_NO_EP;
  data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
END;

procedure blackrokadepanjang(VAR data:tdata);
BEGIN
  data.hashkey:=data.hashkey xor hashkey[_BENTENGHITAM,a8];//hash key menggunakan zorbist key
  data.hashkey:=data.hashkey xor hashkey[_RAJAHITAM,e8];
  data.hashkey:=data.hashkey xor hashkey[_BENTENGHITAM,d8];
  data.hashkey:=data.hashkey xor hashkey[_RAJAHITAM,c8];
  data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];

  data.flagrokade:=data.flagrokade and not BITHITAMNOC;
//  data.flagrokade:=data.flagrokade and not BITHITAMLC;
  data.bc:=ROKADEPANJANG;

  data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];

  data.papan[e8]:=0;
  data.papan[a8]:=0;
  data.papan[d8]:=_BENTENGHITAM;
  data.papan[c8]:=_RAJAHITAM;

//  inc(data.jumlah_langkah);

  data.blackpieces:=data.blackpieces and bit2nmasknot[e8];
  data.blackpieces:=data.blackpieces and bit2nmasknot[a8];
  data.blackpieces:=data.blackpieces or bit2nmask[d8];
  data.blackpieces:=data.blackpieces or bit2nmask[c8];

  data.kingblack:=data.kingblack and bit2nmasknot[e8];
  data.rookblack:=data.rookblack and bit2nmasknot[a8];
  data.rookqueen:=data.rookqueen and bit2nmasknot[a8];
  data.rookblack:=data.rookblack or bit2nmask[d8];
  data.rookqueen:=data.rookqueen or bit2nmask[d8];
  data.kingblack:=data.kingblack or bit2nmask[c8];

  data.allpiecesr90:=data.allpiecesr90 and not bit2nmask90[e8];
  data.allpiecesr90:=data.allpiecesr90 and not bit2nmask90[a8];
  data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[d8];
  data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[c8];
  data.allpiecesa8h1:=data.allpiecesa8h1 and not bit2nmaska8h1[e8];
  data.allpiecesa8h1:=data.allpiecesa8h1 and not bit2nmaska8h1[a8];
  data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[d8];
  data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[c8];

  data.allpiecesh8a1:=data.allpiecesh8a1 and not bit2nmaskh8a1[e8];
  data.allpiecesh8a1:=data.allpiecesh8a1 and not bit2nmaskh8a1[a8];
  data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[d8];
  data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[c8];

  data.allpieces:=data.blackpieces or data.whitepieces;
  data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
  data.ep:=_NO_EP;
  data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
END;

procedure black_ep(var data:tdata;sfrom,sto:integer);
begin
//  dec(data.jum_materi_putih);
  data.move50count:=0;
  dec(data.materialscore,_NILAI_PION);
  IF data.ep>sfrom and 7 THEN
     sto:=sfrom-7
  ELSE
     sto:=sfrom-9;

  data.hashkey:=data.hashkey xor hashkey[_PIONHITAM,sfrom];
  data.hashkey:=data.hashkey xor hashkey[_PIONHITAM,sto];
  data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
  data.ep:=_NO_EP;
  data.hashkey:=data.hashkey xor enpassanthashkey[_NO_EP];
  data.hashkey:=data.hashkey xor hashkey[_PIONPUTIH,sto+8];

  data.papan[sto]:=_PIONHITAM;
  data.papan[sfrom]:=0;
  data.papan[sto+8]:=0;

  data.whitepieces:=data.whitepieces and bit2nmasknot[sto+8];
  data.blackpieces:=data.blackpieces and bit2nmasknot[sfrom];
  data.blackpieces:=data.blackpieces or bit2nmask[sto];

  data.allpieces:=data.allpieces and bit2nmasknot[sto+8];
  data.allpieces:=data.allpieces and bit2nmasknot[sfrom];
  data.allpieces:=data.allpieces or bit2nmask[sto];

  data.allpiecesr90:=data.allpiecesr90 and not bit2nmask90[sto+8];
  data.allpiecesr90:=data.allpiecesr90 and not bit2nmask90[sfrom];
  data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[sto];

  data.allpiecesa8h1:=data.allpiecesa8h1 and not bit2nmaska8h1[sto+8];
  data.allpiecesa8h1:=data.allpiecesa8h1 and not bit2nmaska8h1[sfrom];
  data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[sto];

  data.allpiecesh8a1:=data.allpiecesh8a1 and not bit2nmaskh8a1[sto+8];
  data.allpiecesh8a1:=data.allpiecesh8a1 and not bit2nmaskh8a1[sfrom];
  data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[sto];

  data.pawnblack:=data.pawnblack and bit2nmasknot[sfrom];
  data.pawnwhite:=data.pawnwhite and bit2nmasknot[sto+8];
  data.pawnblack:=data.pawnblack or bit2nmask[sto];

//  inc(data.jumlah_langkah);

end;

procedure white_ep(var data:tdata;sfrom,sto:integer);
begin
//  inc(data.jum_materi_putih);
  data.move50count:=0;
  inc(data.materialscore,_NILAI_PION);
  IF data.ep>sfrom and 7 THEN
     sto:=sfrom+9
  ELSE
     sto:=sfrom+7;

  data.hashkey:=data.hashkey xor hashkey[_PIONPUTIH,sfrom];
  data.hashkey:=data.hashkey xor hashkey[_PIONPUTIH,sto];
  data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
  data.ep:=_NO_EP;
  data.hashkey:=data.hashkey xor hashkey[_PIONHITAM,sto-8];
  data.hashkey:=data.hashkey xor enpassanthashkey[_NO_EP];

  data.papan[sto]:=_PIONPUTIH;
  data.papan[sfrom]:=0;
  data.papan[sto-8]:=0;

  data.blackpieces:=data.blackpieces and bit2nmasknot[sto-8];
  data.whitepieces:=data.whitepieces and bit2nmasknot[sfrom];
  data.whitepieces:=data.whitepieces or bit2nmask[sto];

  data.allpieces:=data.allpieces and bit2nmasknot[sto-8];
  data.allpieces:=data.allpieces and bit2nmasknot[sfrom];
  data.allpieces:=data.allpieces or bit2nmask[sto];

  data.allpiecesr90:=data.allpiecesr90 and not bit2nmask90[sto-8];
  data.allpiecesr90:=data.allpiecesr90 and not bit2nmask90[sfrom];
  data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[sto];

  data.allpiecesa8h1:=data.allpiecesa8h1 and not bit2nmaska8h1[sto-8];
  data.allpiecesa8h1:=data.allpiecesa8h1 and not bit2nmaska8h1[sfrom];
  data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[sto];

  data.allpiecesh8a1:=data.allpiecesh8a1 and not bit2nmaskh8a1[sto-8];
  data.allpiecesh8a1:=data.allpiecesh8a1 and not bit2nmaskh8a1[sfrom];
  data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[sto];

  data.pawnwhite:=data.pawnwhite and bit2nmasknot[sfrom];
  data.pawnblack:=data.pawnblack and bit2nmasknot[sto-8];
  data.pawnwhite:=data.pawnwhite or bit2nmask[sto];

end;

procedure promosi_putih(var data:tdata;sto,promosi:integer);
begin
  data.pawnwhite:=data.pawnwhite and not bit2nmask[sto];
  dec(data.materialscore,_NILAI_PION);
  data.hashkey:=data.hashkey xor hashkey[_PIONPUTIH,sto];
  if promosi=PROMOSI_MENTRI then
  begin
    data.hashkey:=data.hashkey xor hashkey[_MENTRIPUTIH,sto];
    data.papan[sto]:=_MENTRIPUTIH;
    data.queenwhite:=data.queenwhite or bit2nmask[sto];
    data.bishopqueen:=data.bishopqueen or bit2nmask[sto];
    data.rookqueen:=data.rookqueen or bit2nmask[sto];
    inc(data.materialscore,_NILAI_MENTRI);
    inc(data.nilai_perwira_putih,9);
  end else
  if promosi=PROMOSI_KUDA then
  begin
    data.hashkey:=data.hashkey xor hashkey[_KUDAPUTIH,sto];
    data.papan[sto]:=_KUDAPUTIH;
    data.knightwhite:=data.knightwhite or bit2nmask[sto];
    inc(data.materialscore,_NILAI_KUDA);
    inc(data.nilai_perwira_putih,3);
  end else
  if promosi=PROMOSI_BENTENG then
  begin
    data.hashkey:=data.hashkey xor hashkey[_BENTENGPUTIH,sto];
    data.papan[sto]:=_BENTENGPUTIH;
    data.rookwhite:=data.rookwhite or bit2nmask[sto];
    data.rookqueen:=data.rookqueen or bit2nmask[sto];
    inc(data.materialscore,_NILAI_BENTENG);
    inc(data.nilai_perwira_putih,5);
  end else
  if promosi=PROMOSI_GAJAH then
  begin
    data.hashkey:=data.hashkey xor hashkey[_GAJAHPUTIH,sto];
    data.papan[sto]:=_GAJAHPUTIH;
    data.bishopwhite:=data.bishopwhite or bit2nmask[sto];
    data.bishopqueen:=data.bishopqueen or bit2nmask[sto];
    inc(data.materialscore,_NILAI_GAJAH);
    inc(data.nilai_perwira_putih,3);
  end;
end;

procedure promosi_hitam(var data:tdata;sto,promosi:integer);
begin
  data.pawnblack:=data.pawnblack and not bit2nmask[sto];
  inc(data.materialscore,_NILAI_PION);
  data.hashkey:=data.hashkey xor hashkey[_PIONHITAM,sto];
  if promosi=PROMOSI_MENTRI then
  begin
    data.hashkey:=data.hashkey xor hashkey[_MENTRIHITAM,sto];
    data.papan[sto]:=_MENTRIHITAM;
    data.queenblack:=data.queenblack or bit2nmask[sto];
    data.rookqueen:=data.rookqueen or bit2nmask[sto];
    data.bishopqueen:=data.bishopqueen or bit2nmask[sto];
    dec(data.materialscore,_NILAI_MENTRI);
    inc(data.nilai_perwira_HITAM,9);
  end else
  if promosi=PROMOSI_KUDA then
  begin
    data.hashkey:=data.hashkey xor hashkey[_KUDAHITAM,sto];
    data.papan[sto]:=_KUDAHITAM;
    data.knightblack:=data.knightblack or bit2nmask[sto];
    dec(data.materialscore,_NILAI_KUDA);
    inc(data.nilai_perwira_HITAM,3);
  end else
  if promosi=PROMOSI_BENTENG then
  begin
    data.hashkey:=data.hashkey xor hashkey[_BENTENGHITAM,sto];
    data.papan[sto]:=_BENTENGHITAM;
    data.rookblack:=data.rookblack or bit2nmask[sto];
    data.rookqueen:=data.rookqueen or bit2nmask[sto];
    dec(data.materialscore,_NILAI_BENTENG);
    inc(data.nilai_perwira_HITAM,5);
  end else
  if promosi=PROMOSI_GAJAH then
  begin
    data.hashkey:=data.hashkey xor hashkey[_GAJAHHITAM,sto];
    data.papan[sto]:=_GAJAHHITAM;
    data.bishopblack:=data.bishopblack or bit2nmask[sto];
    data.bishopqueen:=data.bishopqueen or bit2nmask[sto];
    dec(data.materialscore,_NILAI_GAJAH);
    inc(data.nilai_perwira_HITAM,3);
  end;
end;


procedure makewhitemove;
var captured_piece,sfrom,sto,promosi:integer;
begin
//cekerror(data);
  inc(total_node);
  sfrom:=moves and 127;
  sto:=(moves shr 7) and 127;
  inc(data.move50count);


//  cekerror(data);
  case sto of
    _ROKADEPENDEK:
    begin
      data.cp:=0;
      whiterokadependek(data);
    end;
    _ROKADEPANJANG:
    begin
      data.cp:=0;
      whiterokadepanjang(data);
    end;
    _EN_PASSANT:
    begin
       white_ep(data,sfrom,sto);
       data.cp:=_PIONPUTIH;
    end;
    else begin
      if data.ep<>_NO_EP then
      begin
        data.hashkey:=(data.hashkey xor enpassanthashkey[data.ep]) xor enpassanthashkey[_NO_EP];
        data.ep:=_NO_EP;
  //      data.hashkey:=data.hashkey xor enpassanthashkey[_NO_EP];
      end;
      captured_piece:=data.papan[sto];

  //    data.hashkey:=data.hashkey xor hashkey[piece,sfrom];
  //    data.hashkey:=data.hashkey xor hashkey[piece,sto];

      IF captured_piece<>0 THEN
      BEGIN
  //       data.hashkey:=data.hashkey xor hashkey[captured_piece,sto];
  //       dec(data.jum_materi_hitam);
         data.move50count:=0;
         data.blackpieces:=data.blackpieces and bit2nmasknot[sto];
         if captured_piece=_PIONHITAM then begin
            data.pawnblack:=data.pawnblack and bit2nmasknot[sto];
            data.hashkey:=data.hashkey xor hashkey[_PIONHITAM,sto];
            data.cp:=_PIONPUTIH;
            inc(data.materialscore,_NILAI_PION);
         end else
         if captured_piece=_KUDAHITAM then begin
            inc(data.materialscore,_NILAI_KUDA);
            data.knightblack:=data.knightblack and bit2nmasknot[sto];
            data.hashkey:=data.hashkey xor hashkey[_KUDAHITAM,sto];
            data.cp:=_KUDAPUTIH;
  //          dec(data.jum_perwira_hitam);
            dec(data.nilai_perwira_hitam,3);
         end else
         if captured_piece=_GAJAHHITAM then begin
            inc(data.materialscore,_NILAI_GAJAH);
            data.bishopblack:=data.bishopblack and bit2nmasknot[sto];
            data.bishopqueen:=data.bishopqueen and bit2nmasknot[sto];
            data.hashkey:=data.hashkey xor hashkey[_GAJAHHITAM,sto];
            data.cp:=_KUDAPUTIH;
  //          data.bishop_oppc:=false;
            dec(data.ngajahhitam);
  //          dec(data.jum_perwira_hitam);
            dec(data.nilai_perwira_hitam,3);
         end else
         if captured_piece=_BENTENGHITAM then begin
            inc(data.materialscore,_NILAI_BENTENG);
            data.rookblack:=data.rookblack and bit2nmasknot[sto];
            data.rookqueen:=data.rookqueen and bit2nmasknot[sto];
            data.hashkey:=data.hashkey xor hashkey[_BENTENGHITAM,sto];
            data.cp:=_BENTENGPUTIH;
  //          dec(data.jum_perwira_hitam);
            dec(data.nilai_perwira_hitam,5);
         end else
         begin
            inc(data.materialscore,_NILAI_MENTRI);
            data.queenblack:=data.queenblack and bit2nmasknot[sto];
            data.rookqueen:=data.rookqueen and bit2nmasknot[sto];
            data.bishopqueen:=data.bishopqueen and bit2nmasknot[sto];
            data.hashkey:=data.hashkey xor hashkey[_MENTRIHITAM,sto];
            data.cp:=_MENTRIPUTIH;
  //          dec(data.jum_perwira_hitam);
            dec(data.nilai_perwira_hitam,9);
         end;
      END//if captured_piece<>0
      ELSE
      begin
      data.cp:=0;
      IF (data.papan[sfrom]=_PIONPUTIH) AND (sto-sfrom=16)
      AND (
      ((data.papan[sto-1]=_PIONHITAM) and (sto and 7 >=1))
      or ( (data.papan[sto+1]=_PIONHITAM) and (sto and 7 <=6) )
      )
      THEN
      BEGIN
        //pion maju 2 petak dan disekitar posisi baru ada pion lawan
        //maka update status en passant
        data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
        data.ep:=sfrom and 7;
        data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
      END;
      end;
      data.papan[sto]:=data.papan[sfrom];
      data.papan[sfrom]:=0;

      data.whitepieces:=data.whitepieces and (bit2nmasknot[sfrom]);
      data.whitepieces:=data.whitepieces or bit2nmask[sto];
      data.allpieces:=data.allpieces and (bit2nmasknot[sfrom]);
      data.allpieces:=data.allpieces or bit2nmask[sto];
      data.allpiecesr90:=data.allpiecesr90 and not (bit2nmask90[sfrom]);
      data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[sto];
      data.allpiecesa8h1:=data.allpiecesa8h1 and not (bit2nmaska8h1[sfrom]);
      data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[sto];
      data.allpiecesh8a1:=data.allpiecesh8a1 and not (bit2nmaskh8a1[sfrom]);
      data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[sto];

      if data.papan[sto]=_PIONPUTIH then
      begin
        data.move50count:=0;
        data.pawnwhite:=data.pawnwhite or bit2nmask[sto];
        data.pawnwhite:=data.pawnwhite and bit2nmasknot[sfrom];
        data.hashkey:=data.hashkey xor hashkey[_PIONPUTIH,sfrom];
        data.hashkey:=data.hashkey xor hashkey[_PIONPUTIH,sto];
        promosi:=(moves shr 14) and 7;
        if (sto>=a8) and (promosi=0) then promosi:=promosi_mentri;
        if promosi<>0 then
        begin
          promosi_putih(data,sto,promosi);
          data.cp:=10;
        end;

      end else
      if data.papan[sto]=_KUDAPUTIH then
      begin
        data.knightwhite:=data.knightwhite or bit2nmask[sto];
        data.knightwhite:=data.knightwhite and bit2nmasknot[sfrom];
        data.hashkey:=data.hashkey xor hashkey[_KUDAPUTIH,sfrom];
        data.hashkey:=data.hashkey xor hashkey[_KUDAPUTIH,sto];

      end else
      if data.papan[sto]=_GAJAHPUTIH then
      begin
        data.bishopwhite:=data.bishopwhite or bit2nmask[sto];
        data.bishopqueen:=data.bishopqueen or bit2nmask[sto];
        data.bishopwhite:=data.bishopwhite and bit2nmasknot[sfrom];
        data.bishopqueen:=data.bishopqueen and bit2nmasknot[sfrom];
        data.hashkey:=data.hashkey xor hashkey[_GAJAHPUTIH,sfrom];
        data.hashkey:=data.hashkey xor hashkey[_GAJAHPUTIH,sto];

      end else
      if data.papan[sto]=_BENTENGPUTIH then
      begin
        data.rookwhite:=data.rookwhite or bit2nmask[sto];
        data.rookqueen:=data.rookqueen or bit2nmask[sto];
        data.rookwhite:=data.rookwhite and bit2nmasknot[sfrom];
        data.rookqueen:=data.rookqueen and bit2nmasknot[sfrom];
        data.hashkey:=data.hashkey xor hashkey[_BENTENGPUTIH,sfrom];
        data.hashkey:=data.hashkey xor hashkey[_BENTENGPUTIH,sto];
        if //(data.wc=NOROKADE) and
         (data.flagrokade and bitputihnoc<>0)
        then
        begin
//          assert(data.wc=norokade);
          if (sfrom=h1) then
          begin
            data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
            data.flagrokade:=data.flagrokade and not BITPUTIHSC;
            data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
          end else
          if (sfrom=a1) then
          begin
            data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
            data.flagrokade:=data.flagrokade and not BITPUTIHLC;
            data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
          end;
        end;
      end else
      if data.papan[sto]=_MENTRIPUTIH then
      begin
        data.queenwhite:=data.queenwhite or bit2nmask[sto];
        data.bishopqueen:=data.bishopqueen or bit2nmask[sto];
        data.rookqueen:=data.rookqueen or bit2nmask[sto];
        data.queenwhite:=data.queenwhite and bit2nmasknot[sfrom];
        data.rookqueen:=data.rookqueen and bit2nmasknot[sfrom];
        data.bishopqueen:=data.bishopqueen and bit2nmasknot[sfrom];
        data.hashkey:=data.hashkey xor hashkey[_MENTRIPUTIH,sfrom];
        data.hashkey:=data.hashkey xor hashkey[_MENTRIPUTIH,sto];

      end else
      if data.papan[sto]=_RAJAPUTIH then
      begin
        data.kingwhite:=data.kingwhite or bit2nmask[sto];
        data.kingwhite:=data.kingwhite and bit2nmasknot[sfrom];
        data.hashkey:=data.hashkey xor hashkey[_RAJAPUTIH,sfrom];
        data.hashkey:=data.hashkey xor hashkey[_RAJAPUTIH,sto];

        if data.flagrokade and bitputihnoc<>0 then
        begin
          data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
          data.flagrokade:=data.flagrokade and not BITPUTIHNOC;
          data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
        end;
      end;
    end;
  end;
//  cekerror(data);

end;

procedure makeblackmove;
var captured_piece,piece:integer;
sfrom,sto,promosi:integer;
begin
  inc(total_node);
  sfrom:=moves and 127;
  sto:=(moves shr 7) and 127;
  inc(data.move50count);
  data.cp:=0;

  case sto of
    _ROKADEPENDEK:
    begin
      blackrokadependek(data);
    end;
    _ROKADEPANJANG:
    begin
      blackrokadepanjang(data);
    end;
    _EN_PASSANT:
    begin
      black_ep(data,sfrom,sto);
      data.cp:=_PIONPUTIH;
    end;
    else
    begin
      piece:=data.papan[sfrom];
      if data.ep<>_NO_EP then
      begin
        data.hashkey:=(data.hashkey xor enpassanthashkey[data.ep]) xor enpassanthashkey[_NO_EP];
        data.ep:=_NO_EP;
  //      data.hashkey:=data.hashkey
      end;
      captured_piece:=data.papan[sto];

      data.hashkey:=data.hashkey xor hashkey[piece,sfrom];
      data.hashkey:=data.hashkey xor hashkey[piece,sto];

      IF captured_piece<>0 THEN
      BEGIN
         data.move50count:=0;
         data.hashkey:=data.hashkey xor hashkey[captured_piece,sto];
  //       dec(data.jum_materi_putih);
         data.whitepieces:=data.whitepieces and bit2nmasknot[sto];
         if captured_piece=_PIONPUTIH then begin
            data.pawnwhite:=data.pawnwhite and bit2nmasknot[sto];
            dec(data.materialscore,_NILAI_PION);
            data.cp:=_PIONPUTIH;
         end else
         if captured_piece=_KUDAPUTIH then begin
            dec(data.materialscore,_NILAI_KUDA);
            data.knightwhite:=data.knightwhite and bit2nmasknot[sto];
  //          dec(data.jum_perwira_putih);
            dec(data.nilai_perwira_putih,3);
            data.cp:=_KUDAPUTIH;
         end else
         if captured_piece=_GAJAHPUTIH then begin
            dec(data.materialscore,_NILAI_GAJAH);
            data.bishopwhite:=data.bishopwhite and bit2nmasknot[sto];
            data.bishopqueen:=data.bishopqueen and bit2nmasknot[sto];
  //          dec(data.jum_perwira_putih);
            dec(data.nilai_perwira_putih,3);
  //          data.bishop_oppc:=false;
            data.cp:=_KUDAPUTIH;
            dec(data.ngajahputih);
         end else
         if captured_piece=_BENTENGPUTIH then begin
            dec(data.materialscore,_NILAI_BENTENG);
            data.rookwhite:=data.rookwhite and bit2nmasknot[sto];
            data.rookqueen:=data.rookqueen and bit2nmasknot[sto];
  //          dec(data.jum_perwira_putih);
            dec(data.nilai_perwira_putih,5);
            data.cp:=_BENTENGPUTIH;
         end else
         begin
            dec(data.materialscore,_NILAI_MENTRI);
            data.queenwhite:=data.queenwhite and bit2nmasknot[sto];
            data.rookqueen:=data.rookqueen and bit2nmasknot[sto];
            data.bishopqueen:=data.bishopqueen and bit2nmasknot[sto];
  //          dec(data.jum_perwira_putih);
            dec(data.nilai_perwira_putih,9);
            data.cp:=_MENTRIPUTIH;
         end;
      END//if captured_piece<>0
      ELSE
      IF (data.papan[sfrom]=_PIONHITAM) AND (sfrom-sto=16)
      AND (
      ((data.papan[sto-1]=_PIONPUTIH) and (sto and 7 >=1))
      or ( (data.papan[sto+1]=_PIONPUTIH) and (sto and 7 <=6) )
      )
      THEN
      BEGIN
        //pion maju 2 petak dan disekitar posisi baru ada pion lawan
        //maka update status en passant
        data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
        data.ep:=sfrom and 7;
        data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
      END;

      data.papan[sto]:=piece;
      data.papan[sfrom]:=0;

      data.blackpieces:=data.blackpieces and (bit2nmasknot[sfrom]);
      data.blackpieces:=data.blackpieces or bit2nmask[sto];
      data.allpieces:=data.allpieces and (bit2nmasknot[sfrom]);
      data.allpieces:=data.allpieces or bit2nmask[sto];
      data.allpiecesr90:=data.allpiecesr90 and not (bit2nmask90[sfrom]);
      data.allpiecesr90:=data.allpiecesr90 or bit2nmask90[sto];
      data.allpiecesa8h1:=data.allpiecesa8h1 and not (bit2nmaska8h1[sfrom]);
      data.allpiecesa8h1:=data.allpiecesa8h1 or bit2nmaska8h1[sto];
      data.allpiecesh8a1:=data.allpiecesh8a1 and not (bit2nmaskh8a1[sfrom]);
      data.allpiecesh8a1:=data.allpiecesh8a1 or bit2nmaskh8a1[sto];

      if piece=_PIONHITAM then
      begin
        data.move50count:=0;
        data.pawnblack:=data.pawnblack or bit2nmask[sto];
        data.pawnblack:=data.pawnblack and bit2nmasknot[sfrom];
        promosi:=(moves shr 14) and 7;
        if (sto<=h1) and (promosi=0) then promosi:=promosi_mentri;
        if promosi<>0 then
        begin
          promosi_hitam(data,sto,promosi);
          data.cp:=11;
        end;

      end else
      if piece=_KUDAHITAM then
      begin
        data.knightblack:=data.knightblack or bit2nmask[sto];
        data.knightblack:=data.knightblack and bit2nmasknot[sfrom];
      end else
      if piece=_GAJAHHITAM then
      begin
        data.bishopblack:=data.bishopblack or bit2nmask[sto];
        data.bishopqueen:=data.bishopqueen or bit2nmask[sto];
        data.bishopblack:=data.bishopblack and bit2nmasknot[sfrom];
        data.bishopqueen:=data.bishopqueen and bit2nmasknot[sfrom];
      end else
      if piece=_BENTENGHITAM then
      begin
        data.rookblack:=data.rookblack or bit2nmask[sto];
        data.rookqueen:=data.rookqueen or bit2nmask[sto];
        data.rookblack:=data.rookblack and bit2nmasknot[sfrom];
        data.rookqueen:=data.rookqueen and bit2nmasknot[sfrom];
        if data.flagrokade and bithitamnoc<>0 then
        begin
          //assert(data.bc=norokade);
          if (sfrom=h8) then
          begin
            data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
            data.flagrokade:=data.flagrokade and not BITHITAMSC;
            data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
          end else
          if (sfrom=a8) then
          begin
            data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
            data.flagrokade:=data.flagrokade and not BITHITAMLC;
            data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
          end;
        end;
      end else
      if piece=_MENTRIHITAM then
      begin
        data.queenblack:=data.queenblack or bit2nmask[sto];
        data.rookqueen:=data.rookqueen or bit2nmask[sto];
        data.bishopqueen:=data.bishopqueen or bit2nmask[sto];
        data.queenblack:=data.queenblack and bit2nmasknot[sfrom];
        data.rookqueen:=data.rookqueen and bit2nmasknot[sfrom];
        data.bishopqueen:=data.bishopqueen and bit2nmasknot[sfrom];
      end else
      if piece=_RAJAHITAM then
      begin
        data.kingblack:=data.kingblack or bit2nmask[sto];
        data.kingblack:=data.kingblack and bit2nmasknot[sfrom];

        if data.flagrokade and bithitamnoc<>0 then
        begin
          data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
          data.flagrokade:=data.flagrokade and not BITHITAMNOC;
          data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
        end;

      end;

    end;
  end;
//cekerror(data);
end;

procedure cekerror(data:tdata);
var piecepos:int64;
a,pos:integer;
begin

 for a:=0 to 63 do
 begin
   if (data.papan[a]=_PIONPUTIH) and (bit2nmask[a] and data.pawnwhite=0) then form1.caption:='error 1';
   if (data.papan[a]=_PIONHITAM) and (bit2nmask[a] and data.pawnblack=0) then form1.caption:='error 2';
   if (data.papan[a]=_KUDAPUTIH) and (bit2nmask[a] and data.knightwhite=0) then form1.caption:='error 30';
   if (data.papan[a]=_GAJAHPUTIH) and (bit2nmask[a] and data.bishopwhite=0) then form1.caption:='error 4';
   if (data.papan[a]=_BENTENGPUTIH) and (bit2nmask[a] and data.rookwhite=0) then form1.caption:='error 5';
   if (data.papan[a]=_MENTRIPUTIH) and (bit2nmask[a] and data.queenwhite=0) then form1.caption:='error 6';
   if (data.papan[a]=_RAJAPUTIH) and (bit2nmask[a] and data.kingwhite=0) then form1.caption:='error 7';
   if (data.papan[a]=_RAJAHITAM) and (bit2nmask[a] and data.kingblack=0) then form1.caption:='error 8';
   if (data.papan[a]=_BENTENGHITAM) and (bit2nmask[a] and data.rookblack=0) then form1.caption:='error 8';
   if (data.papan[a]=_MENTRIHITAM) and (bit2nmask[a] and data.queenblack=0) then form1.caption:='error 8';
   if (data.papan[a]=_GAJAHHITAM) and (bit2nmask[a] and data.bishopblack=0) then form1.caption:='error 8';
   if (data.papan[a]=_KUDAHITAM) and (bit2nmask[a] and data.knightblack=0) then form1.caption:='error 8';

  if (data.papan[a]<>0) and (bit2nmaska8h1[a] and data.allpiecesa8h1=0) then
   begin

     form1.caption:='error 5';
     break;
   end;
   if (data.papan[a]<>0) and (bit2nmask90[a] and data.allpiecesr90=0) then
   begin

     form1.caption:='error 5';
     break;
   end;
   if (data.papan[a]<>0) and (bit2nmaskh8a1[a] and data.allpiecesh8a1=0) then
   begin

     form1.caption:='error 5';
     break;
   end;

 end;


  if data.whitepieces or data.blackpieces<>data.allpieces then
    form1.caption:='error 1';
  if data.pawnwhite or data.knightwhite or data.bishopwhite or data.rookwhite
     or data.queenwhite or data.kingwhite <> data.whitepieces then
     form1.caption:='error 2';
  if data.pawnblack or data.knightblack or data.bishopblack or data.rookblack
     or data.queenblack or data.kingblack <> data.blackpieces then
     form1.caption:='error 3000';

  piecepos:=data.allpieces;
  while piecepos<>0 do
  begin
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if data.papan[pos]=0 then form1.caption:='error4';
    if data.allpiecesr90 and bit2nmask90[pos]=0 then  form1.caption:='error4';
    if data.allpiecesa8h1 and bit2nmaska8h1[pos]=0 then  form1.caption:='error4';
    if data.allpiecesh8a1 and bit2nmaskh8a1[pos]=0 then  form1.caption:='error4';
//    if data.papan[pos]<=0 then showmessage('error 4');
  end;


  piecepos:=data.whitepieces;
  while piecepos<>0 do
  begin
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if data.papan[pos]<=0 then form1.caption:='error4';
    if data.allpiecesr90 and bit2nmask90[pos]=0 then  form1.caption:='error4';
    if data.allpiecesa8h1 and bit2nmaska8h1[pos]=0 then  form1.caption:='error4';
    if data.allpiecesh8a1 and bit2nmaskh8a1[pos]=0 then  form1.caption:='error4';

//    if data.papan[pos]<=0 then showmessage('error 4');
  end;
  piecepos:=data.blackpieces;
  while piecepos<>0 do
  begin
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if data.papan[pos]>=0 then form1.caption:='error5';
    if data.allpiecesr90 and bit2nmask90[pos]=0 then  form1.caption:='error4';
    if data.allpiecesa8h1 and bit2nmaska8h1[pos]=0 then  form1.caption:='error4';
    if data.allpiecesh8a1 and bit2nmaskh8a1[pos]=0 then  form1.caption:='error4';

//    if data.papan[pos]>=0 then showmessage('error 5');
  end;
  piecepos:=data.pawnblack;
  while piecepos<>0 do
  begin
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if data.papan[pos]<>_PIONHITAM then form1.caption:='error6';
//    if data.papan[pos]<>_PIONHITAM then showmessage('error 6');
  end;
  piecepos:=data.knightblack;
  while piecepos<>0 do
  begin
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if data.papan[pos]<>_KUDAHITAM then form1.caption:='error7';
//    if data.papan[pos]<>_KUDAHITAM then showmessage('error 7');
  end;
  piecepos:=data.bishopblack;
  while piecepos<>0 do
  begin
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if data.papan[pos]<>_GAJAHHITAM then form1.caption:='error8';
//    if data.papan[pos]<>_GAJAHHITAM then showmessage('error 8');
  end;
  piecepos:=data.rookblack;
  while piecepos<>0 do
  begin
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if data.papan[pos]<>_BENTENGHITAM then form1.caption:='error9';
//    if data.papan[pos]<>_BENTENGHITAM then showmessage('error 9');
  end;
  piecepos:=data.queenblack;
  while piecepos<>0 do
  begin
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if data.papan[pos]<>_MENTRIHITAM then
    begin
      form1.caption:='error100';exit;
    end;
//    if data.papan[pos]<>_MENTRIHITAM then showmessage('error 10');
  end;
  piecepos:=data.kingblack;
  while piecepos<>0 do
  begin
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
//    temp:=pos;
    if data.papan[pos]<>_RAJAHITAM then
       form1.caption:='error 11';
//    if data.papan[pos]<>_RAJAHITAM then showmessage('error 11');
  end;
  piecepos:=data.pawnwhite;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if data.papan[pos]<>_PIONPUTIH then form1.caption:='error 12';
//    if data.papan[pos]<>_PIONPUTIH then showmessage('error 12');
  end;
  piecepos:=data.knightwhite;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if data.papan[pos]<>_KUDAPUTIH then form1.caption:='error 13';
//    if data.papan[pos]<>_KUDAPUTIH then showmessage('error 13');
  end;
  piecepos:=data.bishopwhite;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if data.papan[pos]<>_GAJAHPUTIH then   form1.caption:='error 14';
//    if data.papan[pos]<>_GAJAHPUTIH then showmessage('error 14');
  end;
  piecepos:=data.rookwhite;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if data.papan[pos]<>_BENTENGPUTIH then
       form1.caption:='error 15';
//    if data.papan[pos]<>_BENTENGPUTIH then showmessage('error 15');
  end;
  piecepos:=data.queenwhite;
  while piecepos<>0 do
  begin
    pos:=firstbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
//    if data.papan[pos]<>_MENTRIPUTIH then showmessage('error 16');
    if data.papan[pos]<>_MENTRIPUTIH then
      form1.caption:='error 16';
  end;
  piecepos:=data.kingwhite;
  while piecepos<>0 do
  begin
    pos:=lastbitp(@piecepos);
    piecepos:=piecepos and bit2nmasknot[pos];
    if data.papan[pos]<>_RAJAPUTIH then
      form1.caption:='error 17';
//    if data.papan[pos]<>_RAJAPUTIH then showmessage('error 17');
  end;

end;



end.
