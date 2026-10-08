unit ETC;

interface
uses header;


PROCEDURE updateblackkey(moves:integer;VAR data:tdata);
PROCEDURE updatewhitekey(moves:integer;VAR data:tdata);
procedure updatekey(var data:tdata;moves,giliran:integer);
implementation
uses hashing_header;

procedure updatekey(var data:tdata;moves,giliran:integer);
begin
  if giliran=_SISIPUTIH then
    updatewhitekey(moves,data)
  else
    updateblackkey(moves,data);
end;

PROCEDURE updatewhitekey(moves:integer;VAR data:tdata);
VAR captured_piece,piece,sfrom,sto,promosi:integer;
BEGIN

  sfrom:=moves AND 127;
  sto:=(moves SHR 7) AND 127;
  promosi:=(moves SHR 14) AND 7;
//  cekerror(data);
  IF sto=_ROKADEPENDEK THEN
  BEGIN
    data.hashkey:=data.hashkey xor hashkey[_BENTENGPUTIH,h1];
    data.hashkey:=data.hashkey xor hashkey[_RAJAPUTIH,e1];
    data.hashkey:=data.hashkey xor hashkey[_BENTENGPUTIH,f1];
    data.hashkey:=data.hashkey xor hashkey[_RAJAPUTIH,g1];
    data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
    data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade AND NOT 3];

    data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
    data.hashkey:=data.hashkey xor enpassanthashkey[_NO_EP];
  END ELSE
  IF sto=_ROKADEPANJANG THEN
  BEGIN
    data.hashkey:=data.hashkey xor hashkey[_BENTENGPUTIH,a1];
    data.hashkey:=data.hashkey xor hashkey[_RAJAPUTIH,e1];
    data.hashkey:=data.hashkey xor hashkey[_BENTENGPUTIH,d1];
    data.hashkey:=data.hashkey xor hashkey[_RAJAPUTIH,c1];
    data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
    data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade AND NOT 3];

    data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
    data.hashkey:=data.hashkey xor enpassanthashkey[_NO_EP];
  END ELSE
  IF sto=_EN_PASSANT THEN
  BEGIN
    data.hashkey:=data.hashkey xor hashkey[_PIONPUTIH,sfrom];
    data.hashkey:=data.hashkey xor hashkey[_PIONPUTIH,sto];
    data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
    data.hashkey:=data.hashkey xor hashkey[_PIONHITAM,sto-8];
    data.hashkey:=data.hashkey xor enpassanthashkey[_NO_EP];
  END ELSE
  BEGIN
    data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
    data.hashkey:=data.hashkey xor enpassanthashkey[_NO_EP];
    piece:=data.papan[sfrom];
    captured_piece:=data.papan[sto];

    data.hashkey:=data.hashkey xor hashkey[piece,sfrom];
    data.hashkey:=data.hashkey xor hashkey[piece,sto];

    IF captured_piece<>0 THEN
    BEGIN
       data.hashkey:=data.hashkey xor hashkey[captured_piece,sto];
    END//IF captured_piece<>0
    ELSE
    IF (data.papan[sfrom]=_PIONPUTIH) AND (sto-sfrom=16)
    AND (
    ((data.papan[sto-1]=_PIONHITAM) AND (sTO mod 8 >=1))
    OR ( (data.papan[sto+1]=_PIONHITAM) AND (sTO mod 8 <=6) )
    )
    THEN
    BEGIN
      //pion maju 2 petak dan disekitar posisi baru ada pion lawan
      //maka update status en passant
      data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
      data.hashkey:=data.hashkey xor enpassanthashkey[sfrom mod 8];
    END;

    IF data.papan[sfrom]=_BENTENGPUTIH THEN
    BEGIN
        IF (sfrom=h1) THEN
        BEGIN
          data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
          data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade AND NOT BITPUTIHSC];
        END ELSE
        IF (sfrom=a1) THEN
        BEGIN
          data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
          data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade AND NOT BITPUTIHLC];
        END;
    END ELSE
    IF data.papan[sfrom]=_RAJAPUTIH THEN
    BEGIN
        data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
        data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade AND NOT 3];

    END;
    IF promosi<>0 THEN
    BEGIN
      data.hashkey:=data.hashkey xor hashkey[_PIONPUTIH,sto];
      IF promosi=PROMOSI_MENTRI THEN
        data.hashkey:=data.hashkey xor hashkey[_MENTRIPUTIH,sto] ELSE
      IF promosi=PROMOSI_KUDA THEN
        data.hashkey:=data.hashkey xor hashkey[_KUDAPUTIH,sto] ELSE
      IF promosi=PROMOSI_BENTENG THEN
        data.hashkey:=data.hashkey xor hashkey[_BENTENGPUTIH,sto] ELSE
      IF promosi=PROMOSI_GAJAH THEN
        data.hashkey:=data.hashkey xor hashkey[_GAJAHPUTIH,sto];
    END;
  END;

{  IF data.hashkey<>hashvalue(data) THEN
   form3.memo1.lines.add('error 50');
 }
//  cekerror(data);

END;



PROCEDURE updateblackkey(moves:integer;VAR data:tdata);
VAR captured_piece,piece:integer;
sfrom,sto,promosi:integer;
BEGIN
  sfrom:=moves AND 127;
  sto:=(moves SHR 7) AND 127;
  promosi:=(moves SHR 14) AND 7;

  IF sto=_ROKADEPENDEK THEN
  BEGIN
    data.hashkey:=data.hashkey xor hashkey[_BENTENGHITAM,h8];//hash key menggunakan zorbist key
    data.hashkey:=data.hashkey xor hashkey[_RAJAHITAM,e8];
    data.hashkey:=data.hashkey xor hashkey[_BENTENGHITAM,f8];
    data.hashkey:=data.hashkey xor hashkey[_RAJAHITAM,g8];
    data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
    data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade AND NOT 12];
    data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
    data.hashkey:=data.hashkey xor enpassanthashkey[_NO_EP];
  END ELSE
  IF sto=_ROKADEPANJANG THEN
  BEGIN
    data.hashkey:=data.hashkey xor hashkey[_BENTENGHITAM,a8];//hash key menggunakan zorbist key
    data.hashkey:=data.hashkey xor hashkey[_RAJAHITAM,e8];
    data.hashkey:=data.hashkey xor hashkey[_BENTENGHITAM,d8];
    data.hashkey:=data.hashkey xor hashkey[_RAJAHITAM,c8];
    data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
    data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade AND NOT 12];
    data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
    data.hashkey:=data.hashkey xor enpassanthashkey[_NO_EP];
  END ELSE
  IF sto=_EN_PASSANT THEN
  BEGIN
    data.hashkey:=data.hashkey xor hashkey[_PIONHITAM,sfrom];
    data.hashkey:=data.hashkey xor hashkey[_PIONHITAM,sto];
    data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
    data.hashkey:=data.hashkey xor enpassanthashkey[_NO_EP];
    data.hashkey:=data.hashkey xor hashkey[_PIONPUTIH,sto+8];
  END ELSE
  BEGIN
    piece:=data.papan[sfrom];
    data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
    data.hashkey:=data.hashkey xor enpassanthashkey[_NO_EP];
    captured_piece:=data.papan[sto];
    
    data.hashkey:=data.hashkey xor hashkey[piece,sfrom];
    data.hashkey:=data.hashkey xor hashkey[piece,sto];

    IF captured_piece<>0 THEN
    BEGIN
       data.hashkey:=data.hashkey xor hashkey[captured_piece,sto];
    END//IF captured_piece<>0
    ELSE
    IF (data.papan[sfrom]=_PIONHITAM) AND (sfrom-sto=16)
    AND (
    ((data.papan[sto-1]=_PIONPUTIH) AND (sTO mod 8 >=1))
    OR ( (data.papan[sto+1]=_PIONPUTIH) AND (sTO mod 8 <=6) )
    )
    THEN
    BEGIN
      //pion maju 2 petak dan disekitar posisi baru ada pion lawan
      //maka update status en passant
      data.hashkey:=data.hashkey xor enpassanthashkey[data.ep];
      data.hashkey:=data.hashkey xor enpassanthashkey[sfrom mod 8];
    END;

    IF data.papan[sfrom]=_BENTENGHITAM THEN
    BEGIN
        IF (sfrom=h8) THEN
        BEGIN
          data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
          data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade AND NOT BITHITAMSC];
        END ELSE
        IF (sfrom=a8) THEN
        BEGIN
          data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
          data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade AND NOT BITHITAMLC];
        END;
    END ELSE
    IF data.papan[sfrom]=_RAJAHITAM THEN
    BEGIN
        data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade];
        data.hashkey:=data.hashkey xor rokadehashkey[data.flagrokade AND NOT 12];
    END;
    IF promosi<>0 THEN
    BEGIN
      data.hashkey:=data.hashkey xor hashkey[_PIONHITAM,sto];
      IF promosi=PROMOSI_MENTRI THEN
        data.hashkey:=data.hashkey xor hashkey[_MENTRIHITAM,sto] ELSE
      IF promosi=PROMOSI_KUDA THEN
        data.hashkey:=data.hashkey xor hashkey[_KUDAHITAM,sto] ELSE
      IF promosi=PROMOSI_BENTENG THEN
        data.hashkey:=data.hashkey xor hashkey[_BENTENGHITAM,sto] ELSE
      IF promosi=PROMOSI_GAJAH THEN
        data.hashkey:=data.hashkey xor hashkey[_GAJAHHITAM,sto];
    END;
  END;
END;



end.
