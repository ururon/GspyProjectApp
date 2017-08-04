/*!---------------------------------------------------------
* Copyright (C) Genius Corporation. All rights reserved.
*--------------------------------------------------------*/

var errorCode = {
	NotFound : 0x1001,
	SetParameterError : 0x1002,
	GetParameterError : 0x1003,
	OsError : 0x1004,
	PathErrorFileNotFound : 0x1005,
	FwFileError : 0x1006,
	WriteFwFileError : 0x1007,
	DownloadFileError : 0x1008,
	TimeoutError : 0x1009,
	CRCError : 0x100a,
	FormatError : 0x100b,
	IsBusy : 0x100c,
	NotSupport : 0x100d,
	ValidateError : 0x1010,
	UnknownError : 0x1011
};

exports.ErrorCode = errorCode;