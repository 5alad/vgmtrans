#include "stdafx.h"
#include "KonamiSnesScanner.h"
#include "KonamiSnesSeq.h"
#include "KonamiSnesInstr.h"
#include "SNESDSP.h"

//; Ganbare Goemon 4
//13d3: 8f 1e 06  mov   $06,#$1e
//13d6: 8f 00 0a  mov   $0a,#$00
//13d9: 8f 39 0b  mov   $0b,#$39          ; set header address $3900 to $0a/b
//13dc: cd 00     mov   x,#$00
BytePattern KonamiSnesScanner::ptnSetSongHeaderAddress(
	"\x8f\x1e\x06\x8f\x00\x0a\x8f\x39"
	"\x0b\xcd\x00"
	,
	"x?xx?xx?"
	"xxx"
	,
	11);

//; Axelay SPC
//1948: e4 0c     mov   a,$0c             ; song index (1 origin)
//194a: 8f e6 04  mov   $04,#$e6
//194d: 8f 03 05  mov   $05,#$03          ; $04/5 = $03e6 (sequence header table)
//1950: 9c        dec   a
//1951: 8d 05     mov   y,#$05            ; 5 bytes
//1953: cf        mul   ya
//1954: 7a 04     addw  ya,$04
//1956: da 04     movw  $04,ya
//1958: 8d 00     mov   y,#$00
//195a: cd 00     mov   x,#$00
//195c: f7 04     mov   a,($04)+y
//195e: c4 20     mov   $20,a             ; offset +0: ?
//1960: fc        inc   y
//1961: f7 04     mov   a,($04)+y
//1963: c4 06     mov   $06,a             ; offset +1: ?
//1965: e4 0c     mov   a,$0c
//1967: 68 4d     cmp   a,#$4d
//1969: b0 5a     bcs   $19c5             ; branch if song >= 77
//196b: cd 0c     mov   x,#$0c
//196d: 68 41     cmp   a,#$41
//196f: b0 4a     bcs   $19bb             ; branch if song >= 65
BytePattern KonamiSnesScanner::ptnReadSongListAXE(
	"\xe4\x0c\x8f\xe6\x04\x8f\x03\x05"
	"\x9c\x8d\x05\xcf\x7a\x04\xda\x04"
	"\x8d\x00\xcd\x00\xf7\x04\xc4\x20"
	"\xfc\xf7\x04\xc4\x06\xe4\x0c\x68"
	"\x4d\xb0\x5a\xcd\x0c\x68\x41\xb0"
	"\x4a"
	,
	"x?x??x??"
	"xxxxx?x?"
	"xxxxx?x?"
	"xx?x?x?x"
	"?x?x?x?x"
	"?"
	,
	41);

//; Contra 3 SPC
//197a: e4 0c     mov   a,$0c             ; song index (1 origin)
//197c: 8f e6 04  mov   $04,#$e6
//197f: 8f 03 05  mov   $05,#$03          ; sequence header table address
//1982: 9c        dec   a
//1983: 8d 05     mov   y,#$05            ; 5 bytes
//1985: cf        mul   ya
//1986: 7a 04     addw  ya,$04
//1988: da 04     movw  $04,ya
//198a: 8d 00     mov   y,#$00
//198c: cd 00     mov   x,#$00
//198e: f7 04     mov   a,($04)+y
//1990: c4 20     mov   $20,a             ; offset +0
//1992: fc        inc   y
//1993: f7 04     mov   a,($04)+y
//1995: c4 06     mov   $06,a             ; offset +1
//1997: e4 0c     mov   a,$0c
//1999: 68 60     cmp   a,#$60
//199b: b0 64     bcs   $1a01
//199d: 68 5c     cmp   a,#$5c
//199f: 90 0a     bcc   $19ab
BytePattern KonamiSnesScanner::ptnReadSongListCNTR3(
	"\xe4\x0c\x8f\xe6\x04\x8f\x03\x05"
	"\x9c\x8d\x05\xcf\x7a\x04\xda\x04"
	"\x8d\x00\xcd\x00\xf7\x04\xc4\x20"
	"\xfc\xf7\x04\xc4\x06\xe4\x0c\x68"
	"\x60\xb0\x64\x68\x5c\x90\x0a"
	,
	"x?x??x??"
	"xxxxx?x?"
	"xxxxx?x?"
	"xx?x?x?x"
	"?x?x?x?"
	,
	39);

//; Ganbare Goemon 4
//; dispatch vcmd (e0-ff)
//1947: 1c        asl   a
//1948: fd        mov   y,a
//1949: f6 bc 1a  mov   a,$1abc+y
//194c: 2d        push  a
//194d: f6 bb 1a  mov   a,$1abb+y
//1950: 2d        push  a                 ; push vcmd func address, as a return address
//1951: f6 fb 1a  mov   a,$1afb+y
//1954: f0 08     beq   $195e
BytePattern KonamiSnesScanner::ptnJumpToVcmd(
	"\x1c\xfd\xf6\xbc\x1a\x2d\xf6\xbb"
	"\x1a\x2d\xf6\xfb\x1a\xf0\x08"
	,
	"xxx??xx?"
	"?xx??x?"
	,
	15);

//; Contra 3 SPC
//0e61: 80        setc
//0e62: a4 04     sbc   a,$04
//0e64: 1c        asl   a
//0e65: fd        mov   y,a
//0e66: f6 de 0d  mov   a,$0dde+y
//0e69: 2d        push  a
//0e6a: f6 dd 0d  mov   a,$0ddd+y
//0e6d: 2d        push  a
//0e6e: dd        mov   a,y
//0e6f: 5c        lsr   a
//0e70: fd        mov   y,a
//0e71: f6 27 0e  mov   a,$0e27+y
//0e74: f0 08     beq   $0e7e
BytePattern KonamiSnesScanner::ptnJumpToVcmdCNTR3(
	"\x80\xa4\x04\x1c\xfd\xf6\xde\x0d"
	"\x2d\xf6\xdd\x0d\x2d\xdd\x5c\xfd"
	"\xf6\x27\x0e\xf0\x08"
	,
	"xx?xxx??"
	"xx??xxxx"
	"x??x?"
	,
	21);

//; Madara 2
//0e5b: e4 08     mov   a,$08
//0e5d: 8f de 04  mov   $04,#$de
//0e60: 68 e0     cmp   a,#$e0
//0e62: b0 0c     bcs   $0e70             ; branch if vcmd e0..ff
//0e64: 8f 60 04  mov   $04,#$60
//0e67: 68 62     cmp   a,#$62
//0e69: 90 05     bcc   $0e70             ; branch if vcmd 60..61
BytePattern KonamiSnesScanner::ptnBranchForVcmd6xMDR2(
	"\xe4\x08\x8f\xde\x04\x68\xe0\xb0"
	"\x0c\x8f\x60\x04\x68\x62\x90\x05"
	,
	"x?x??xxx"
	"?x??xxx?"
	,
	15);

//; Contra 3 SPC
//0e4c: e4 08     mov   a,$08
//0e4e: 8f db 04  mov   $04,#$db
//0e51: 68 e0     cmp   a,#$e0
//0e53: b0 0c     bcs   $0e61
//0e55: 68 65     cmp   a,#$65
//0e57: 90 05     bcc   $0e5e
BytePattern KonamiSnesScanner::ptnBranchForVcmd6xCNTR3(
	"\xe4\x08\x8f\xdb\x04\x68\xe0\xb0"
	"\x0c\x68\x65\x90\x05"
	,
	"x?x??xxx"
	"?xx??"
	,
	13);

//; Ganbare Goemon 4
//0266: 8f 5d f2  mov   $f2,#$5d
//0269: 8f 04 f3  mov   $f3,#$04          ; source dir = $0400
BytePattern KonamiSnesScanner::ptnSetDIR(
	"\x8f\x5d\xf2\x8f\x04\xf3"
	,
	"xxxx?x"
	,
	6);

//; Contra 3 SPC
//0919: e8 50     mov   a,#$50
//091b: 8d 5d     mov   y,#$5d
//091d: cc f2 00  mov   $00f2,y
//0920: c5 f3 00  mov   $00f3,a           ; DIR = $5000
BytePattern KonamiSnesScanner::ptnSetDIRCNTR3(
	"\xe8\x50\x8d\x5d\xcc\xf2\x00\xc5"
	"\xf3\x00"
	,
	"x?xxxxxx"
	"xx"
	,
	10);

//; Ganbare Goemon 4
//; vcmd e2 - set instrument
//1b95: 09 11 10  or    ($10),($11)
//1b98: fd        mov   y,a
//1b99: f5 a1 01  mov   a,$01a1+x
//1b9c: d0 27     bne   $1bc5
//1b9e: dd        mov   a,y
//1b9f: 68 28     cmp   a,#$28
//1ba1: b0 0c     bcs   $1baf             ; use another map if patch number is large
//1ba3: 8f 3c 04  mov   $04,#$3c
//1ba6: 8f 0a 05  mov   $05,#$0a          ; common sample map = #$0a3c
//1ba9: 3f ee 1b  call  $1bee
//1bac: 5f e2 18  jmp   $18e2
//; use another map
//1baf: a8 28     sbc   a,#$28            ; patch -= 0x28
//1bb1: 2d        push  a
//1bb2: eb 25     mov   y,$25             ; bank offset
//1bb4: f6 20 0a  mov   a,$0a20+y
//1bb7: c4 04     mov   $04,a
//1bb9: f6 21 0a  mov   a,$0a21+y
//1bbc: c4 05     mov   $05,a             ; sample map = *(u16)($0a20 + bank_offset)
//1bbe: ae        pop   a
//1bbf: 3f ee 1b  call  $1bee
//1bc2: 5f e2 18  jmp   $18e2
BytePattern KonamiSnesScanner::ptnLoadInstr(
	"\x09\x11\x10\xfd\xf5\xa1\x01\xd0"
	"\x27\xdd\x68\x28\xb0\x0c\x8f\x3c"
	"\x04\x8f\x0a\x05\x3f\xee\x1b\x5f"
	"\xe2\x18\xa8\x28\x2d\xeb\x25\xf6"
	"\x20\x0a\xc4\x04\xf6\x21\x0a\xc4"
	"\x05\xae\x3f\xee\x1b\x5f\xe2\x18"
	,
	"x??xx??x"
	"?xx?xxx?"
	"?x??x??x"
	"??x?xx?x"
	"??x?x??x"
	"?xx??x??"
	,
	48);

//; Contra 3 SPC
//; vcmd e2 - set instrument
//0ef1: 09 20 17  or    ($17),($20)
//0ef4: d5 97 02  mov   $0297+x,a         ; instrument #
//0ef7: 68 19     cmp   a,#$19
//0ef9: b0 04     bcs   $0eff             ; branch if patch number >= 25
//0efb: 68 14     cmp   a,#$14
//0efd: b0 11     bcs   $0f10             ; branch if patch number >= 20
//0eff: e8 36     mov   a,#$36
//0f01: c4 04     mov   $04,a
//0f03: e8 06     mov   a,#$06
//0f05: c4 05     mov   $05,a             ; common sample map = $0636
//0f07: f5 97 02  mov   a,$0297+x
//0f0a: 3f 36 0f  call  $0f36
//0f0d: 5f 8c 0c  jmp   $0c8c
//; use another map
//0f10: e5 05 02  mov   a,$0205           ; bank number
//0f13: 1c        asl   a
//0f14: fd        mov   y,a
//0f15: f6 28 06  mov   a,$0628+y
//0f18: c4 04     mov   $04,a
//0f1a: f6 29 06  mov   a,$0629+y
//0f1d: c4 05     mov   $05,a             ; sample map = *(u16)($0628 + bank * 2)
//0f1f: f5 97 02  mov   a,$0297+x         ; instrument #
//0f22: 80        setc
//0f23: a8 14     sbc   a,#$14            ; patch -= 0x14
//0f25: 3f 36 0f  call  $0f36
//0f28: 5f 8c 0c  jmp   $0c8c
//; update sample (percussion note)
//0f2b: e8 fe     mov   a,#$fe
//0f2d: c4 04     mov   $04,a
//0f2f: e8 06     mov   a,#$06
//0f31: c4 05     mov   $05,a             ; $04/5 = $06fe
////; load instrument attributes from instrument table
////; a = patch number, $04 = instrument table address
//0f33: f5 97 02  mov   a,$0297+x
//0f36: 8d 08     mov   y,#$08
//0f38: cf        mul   ya
//0f39: 7a 04     addw  ya,$04
//0f3b: da 04     movw  $04,ya            ; load address by index `$04 += (patch * 8)`
BytePattern KonamiSnesScanner::ptnLoadInstrCNTR3(
	"\x09\x20\x17\xd5\x97\x02\x68\x19"
	"\xb0\x04\x68\x14\xb0\x11\xe8\x36"
	"\xc4\x04\xe8\x06\xc4\x05\xf5\x97"
	"\x02\x3f\x36\x0f\x5f\x8c\x0c\xe5"
	"\x05\x02\x1c\xfd\xf6\x28\x06\xc4"
	"\x04\xf6\x29\x06\xc4\x05\xf5\x97"
	"\x02\x80\xa8\x14\x3f\x36\x0f\x5f"
	"\x8c\x0c\xe8\xfe\xc4\x04\xe8\x06"
	"\xc4\x05\xf5\x97\x02\x8d\x08\xcf"
	"\x7a\x04\xda\x04"
	,
	"x??x??x?"
	"xxx?xxx?"
	"x?x?x?x?"
	"?x??x??x"
	"??xxx??x"
	"?x??x?x?"
	"?xx?x??x"
	"??x?x?x?"
	"x?x??xxx"
	"x?x?"
	,
	76);

//; Ganbare Goemon 4
//; load instrument for percussive note
//1be8: 8f e6 04  mov   $04,#$e6
//1beb: 8f 0d 05  mov   $05,#$0d          ; $04 = #$0de6
//; load instrument attributes from instrument table
//; a = patch number, $04 = instrument table address
//1bee: 8d 07     mov   y,#$07
//1bf0: cf        mul   ya
//1bf1: 7a 04     addw  ya,$04
//1bf3: da 04     movw  $04,ya            ; load address by index `$04 += (patch * 7)`
BytePattern KonamiSnesScanner::ptnLoadPercInstr(
	"\x8f\xe6\x04\x8f\x0d\x05\x8d\x07"
	"\xcf\x7a\x04\xda\x04"
	,
	"x??x??xx"
	"xx?x?"
	,
	13);

void KonamiSnesScanner::Scan(RawFile* file, void* info)
{
	uint32_t nFileLength = file->size();
	if (nFileLength == 0x10000)
	{
		SearchForKonamiSnesFromARAM(file);
	}
	else
	{
		SearchForKonamiSnesFromROM(file);
	}
	return;
}

void KonamiSnesScanner::SearchForKonamiSnesFromARAM (RawFile* file)
{
	KonamiSnesVersion version = KONAMISNES_NONE;

	bool hasSongList;

	std::wstring basefilename = RawFile::removeExtFromPath(file->GetFileName());
	std::wstring name = file->tag.HasTitle() ? file->tag.title : basefilename;

	// find a song header
	UINT ofsSetSongHeaderAddress;
	UINT ofsReadSongList;
	uint16_t addrSongHeader;
	uint16_t addrSongList;
	int8_t primarySongIndex;
	if (file->SearchBytePattern(ptnSetSongHeaderAddress, ofsSetSongHeaderAddress)) {
		addrSongHeader = file->GetByte(ofsSetSongHeaderAddress + 4) | (file->GetByte(ofsSetSongHeaderAddress + 7) << 8);
		hasSongList = false;
	}
	else if (file->SearchBytePattern(ptnReadSongListAXE, ofsReadSongList)) {
		addrSongList = file->GetByte(ofsReadSongList + 3) | (file->GetByte(ofsReadSongList + 6) << 8);
		primarySongIndex = file->GetByte(ofsReadSongList + 32);
		hasSongList = true;
	}
	else if (file->SearchBytePattern(ptnReadSongListCNTR3, ofsReadSongList)) {
		addrSongList = file->GetByte(ofsReadSongList + 3) | (file->GetByte(ofsReadSongList + 6) << 8);
		primarySongIndex = file->GetByte(ofsReadSongList + 32);
		hasSongList = true;
	}
	else {
		return;
	}

	// find the vcmd length table
	UINT ofsJumpToVcmd;
	uint16_t addrVcmdLengthTable;
	uint8_t vcmd6XCountInList;
	if (file->SearchBytePattern(ptnJumpToVcmd, ofsJumpToVcmd)) {
		addrVcmdLengthTable = file->GetShort(ofsJumpToVcmd + 11);
		vcmd6XCountInList = 0;

		// check table length
		if (addrVcmdLengthTable + vcmd6XCountInList + 0x20 >= 0x10000) {
			return;
		}
	}
	else if (file->SearchBytePattern(ptnJumpToVcmdCNTR3, ofsJumpToVcmd)) {
		addrVcmdLengthTable = file->GetShort(ofsJumpToVcmd + 17);

		UINT ofsBranchForVcmd6x;
		if (file->SearchBytePattern(ptnBranchForVcmd6xCNTR3, ofsBranchForVcmd6x)) {
			// vcmd 60-64 is in the list
			vcmd6XCountInList = 5;
		}
		else if (file->SearchBytePattern(ptnBranchForVcmd6xMDR2, ofsBranchForVcmd6x)) {
			// vcmd 60-61 is in the list
			vcmd6XCountInList = 2;
		}
		else {
			return;
		}

		// check table length
		if (addrVcmdLengthTable + vcmd6XCountInList + 0x20 >= 0x10000) {
			return;
		}
	}
	else {
		return;
	}

	// detect revision by vcmd length
	if (hasSongList) {
		if (vcmd6XCountInList == 5) {
			version = KONAMISNES_V1;
		}
		else {
			version = KONAMISNES_V2;
		}
	}
	else {
		assert(vcmd6XCountInList == 0);
		if (file->GetByte(addrVcmdLengthTable + (0xed - 0xe0)) == 3) {
			version = KONAMISNES_V3;
		}
		else if (file->GetByte(addrVcmdLengthTable + (0xfc - 0xe0)) == 2) {
			version = KONAMISNES_V4;
		}
		else {
			version = KONAMISNES_V5;
		}
	}

	// load song(s)
	if (hasSongList) {
		// TODO: song index search
		int8_t songIndex = primarySongIndex;

		uint32_t addrSongHeaderPtr = addrSongList + songIndex * 5;
		if (addrSongHeaderPtr + 5 > 0x10000) {
			return;
		}

		addrSongHeader = file->GetShort(addrSongHeaderPtr + 3);
	}

	KonamiSnesSeq* newSeq = new KonamiSnesSeq(file, version, addrSongHeader, name);
	if (!newSeq->LoadVGMFile()) {
		delete newSeq;
		return;
	}

	// scan for DIR address
	UINT ofsSetDIR;
	uint16_t spcDirAddr;
	std::map<std::wstring, std::vector<uint8_t>>::iterator itrDSP;
	if (file->SearchBytePattern(ptnSetDIR, ofsSetDIR)) {
		spcDirAddr = file->GetByte(ofsSetDIR + 4) << 8;
	}
	else if (file->SearchBytePattern(ptnSetDIRCNTR3, ofsSetDIR)) {
		spcDirAddr = file->GetByte(ofsSetDIR + 1) << 8;
	}
	else if ((itrDSP = file->tag.binaries.find(L"dsp")) != file->tag.binaries.end()) {
		// read DIR from SPC700 snapshot
		spcDirAddr = itrDSP->second[0x5d] << 8;
	}
	else {
		return;
	}

	// scan for instrument table
	UINT ofsLoadInstr;
	uint16_t addrCommonInstrTable;
	uint16_t addrBankedInstrTable;
	uint8_t firstBankedInstr;
	uint16_t addrPercInstrTable;
	if (file->SearchBytePattern(ptnLoadInstr, ofsLoadInstr)) {
		addrCommonInstrTable = file->GetByte(ofsLoadInstr + 15) | (file->GetByte(ofsLoadInstr + 18) << 8);
		firstBankedInstr = file->GetByte(ofsLoadInstr + 11);

		uint8_t addrCurrentBank = file->GetByte(ofsLoadInstr + 30);
		uint16_t addrInstrTableBanks = file->GetShort(ofsLoadInstr + 32);
		addrBankedInstrTable = file->GetShort(addrInstrTableBanks + file->GetByte(addrCurrentBank));

		// scan for percussive instrument table
		UINT ofsLoadPercInstr;
		if (file->SearchBytePattern(ptnLoadPercInstr, ofsLoadPercInstr)) {
			addrPercInstrTable = file->GetByte(ofsLoadPercInstr + 1) | (file->GetByte(ofsLoadPercInstr + 4) << 8);
		}
		else {
			return;
		}
	}
	else if (file->SearchBytePattern(ptnLoadInstrCNTR3, ofsLoadInstr)) {
		addrCommonInstrTable = file->GetByte(ofsLoadInstr + 15) | (file->GetByte(ofsLoadInstr + 19) << 8);
		firstBankedInstr = file->GetByte(ofsLoadInstr + 11);

		uint16_t addrCurrentBank = file->GetShort(ofsLoadInstr + 32);
		uint16_t addrInstrTableBanks = file->GetShort(ofsLoadInstr + 37);
		addrBankedInstrTable = file->GetShort(addrInstrTableBanks + file->GetByte(addrCurrentBank) * 2);

		addrPercInstrTable = file->GetByte(ofsLoadInstr + 59) | (file->GetByte(ofsLoadInstr + 63) << 8);
	}
	else {
		return;
	}

	KonamiSnesInstrSet * newInstrSet = new KonamiSnesInstrSet(file, version, addrCommonInstrTable, addrBankedInstrTable, firstBankedInstr, addrPercInstrTable, spcDirAddr);
	if (!newInstrSet->LoadVGMFile()) {
		delete newInstrSet;
		return;
	}
}

void KonamiSnesScanner::SearchForKonamiSnesFromROM (RawFile* file)
{
}