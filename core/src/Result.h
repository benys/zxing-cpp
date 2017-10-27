#pragma once
/*
* Copyright 2016 Nu-book Inc.
* Copyright 2016 ZXing authors
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include "ByteArray.h"
#include "BarcodeFormat.h"
#include "ResultPoint.h"
#include "ResultMetadata.h"
#include "DecoderResult.h"
#include "DecodeStatus.h"

#include <string>
#include <chrono>
#include <vector>

namespace ZXing {

class ResultMetadata;

/**
* <p>Encapsulates the result of decoding a barcode within an image.</p>
*
* @author Sean Owen
*/
class Result
{
public:
	using time_point = std::chrono::steady_clock::time_point;

	explicit Result(DecodeStatus status) : _status(status) {}

	Result(std::wstring&& text, ByteArray&& rawBytes, std::vector<ResultPoint>&& resultPoints, BarcodeFormat format)
	    : _text(std::move(text)),
	      _rawBytes(std::move(rawBytes)),
		  _resultPoints(std::move(resultPoints)),
	      _format(format)
	{
		_numBits = static_cast<int>(_rawBytes.size()) * 8;
	}

	Result(DecoderResult&& decodeResult, std::vector<ResultPoint>&& resultPoints, BarcodeFormat format)
		: _status(decodeResult.errorCode()),
	      _text(std::move(decodeResult).text()),
		  _rawBytes(std::move(decodeResult).rawBytes()),
	      _numBits(decodeResult.numBits()),
		  _resultPoints(std::move(resultPoints)),
	      _format(format)
	{
		if (!isValid())
			return;

		//TODO: change ResultMetadata::put interface, so we can move from decodeResult?
		const auto& byteSegments = decodeResult.byteSegments();
		if (!byteSegments.empty()) {
			metadata().put(ResultMetadata::BYTE_SEGMENTS, byteSegments);
		}
		const auto& ecLevel = decodeResult.ecLevel();
		if (!ecLevel.empty()) {
			metadata().put(ResultMetadata::ERROR_CORRECTION_LEVEL, ecLevel);
		}
		if (decodeResult.hasStructuredAppend()) {
			metadata().put(ResultMetadata::STRUCTURED_APPEND_SEQUENCE, decodeResult.structuredAppendSequenceNumber());
			metadata().put(ResultMetadata::STRUCTURED_APPEND_PARITY, decodeResult.structuredAppendParity());
		}
		//TODO: what about the other optional data in DecoderResult?
	}

	bool isValid() const {
		return StatusIsOK(_status);
	}

	DecodeStatus status() const {
		return _status;
	}

	const std::wstring& text() const {
		return _text;
	}
	void setText(std::wstring&& text) {
		_text = std::move(text);
	}

	const ByteArray& rawBytes() const {
		return _rawBytes;
	}
	
	int numBits() const {
		return _numBits;
	}

	const std::vector<ResultPoint>& resultPoints() const {
		return _resultPoints;
	}

	void setResultPoints(std::vector<ResultPoint>&& points) {
		_resultPoints = std::move(points);
	}

	void addResultPoints(const std::vector<ResultPoint>& points);

	BarcodeFormat format() const {
		return _format;
	}
	void setFormat(BarcodeFormat format) {
		_format = format;
	}

	time_point timestamp() const {
		return _timestamp;
	}

	const ResultMetadata& metadata() const {
		return _metadata;
	}

	ResultMetadata& metadata() {
		return _metadata;
	}

private:
	DecodeStatus _status = DecodeStatus::NoError;
	std::wstring _text;
	ByteArray _rawBytes;
	int _numBits = 0;
	std::vector<ResultPoint> _resultPoints;
	BarcodeFormat _format = BarcodeFormat::FORMAT_COUNT;
	time_point _timestamp = std::chrono::steady_clock::now();
	ResultMetadata _metadata;
};

} // ZXing
