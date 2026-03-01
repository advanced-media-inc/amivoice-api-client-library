(function () {
	var data_ = new Uint8Array(1 + 27 + 1 + 19 + 27 + 1 + 16 + 27 + 255 + 4096);
	var samplesPerSec_;
	var lookahead_;
	var bytesPerFrame_;
	var samplesPerFrame_;
	var granulePosition_;
	var bitstreamSerialNumber_;
	var pageSequenceNumber_;
	var frames_;
	var bytes_;
	var samples_;

	function convert_(data) {
		if (!data) {
			samplesPerSec_ = 0;
			return data;
		}
		if (data.buffer) {
			return convert__(data);
		}
		var oggOpusData = [];
		var dataIndex = 0;
		if (dataIndex >= data.length) {
			return data;
		}
		var opusData = data[dataIndex++];
		var opusDataIndex = 0;
		var opusDataBytes = opusData.length;
		if (opusDataBytes < 16) {
			return data;
		}
		samplesPerSec_ = opusData[opusDataIndex + 6] | (opusData[opusDataIndex + 7] << 8);
		var channels = opusData[opusDataIndex + 8];
		var complexity = opusData[opusDataIndex + 10];
		lookahead_ = opusData[opusDataIndex + 11];
		if (lookahead_ === 56) {
			lookahead_ = 312;
		}
		bytesPerFrame_ = opusData[opusDataIndex + 12];
		samplesPerFrame_ = opusData[opusDataIndex + 13] | (opusData[opusDataIndex + 14] << 8);
		opusDataIndex += 16;
		opusDataBytes -= 16;
		if (false) {
			console.log("<OpusHeader> samplesPerSec[" + samplesPerSec_ + "(" + this.samplesPerSec + ")]"
								  + " channels[" + channels + "]"
								  + " complexity[" + complexity + "]"
								  + " lookahead[" + lookahead_ + "]"
								  + " bytesPerFrame[" + bytesPerFrame_ + "]"
								  + " samplesPerFrame[" + samplesPerFrame_ + "] </OpusHeader>");
		}
		if (samplesPerSec_ != 48000) {
			return data;
		}
		// <!--
		granulePosition_ = 0;
		bitstreamSerialNumber_ = /* as a random number */ 7584;
		pageSequenceNumber_ = 0;
		var dataBytes = 27;
		var dataBytes2 = 100;
		data_[dataBytes2++] = 0x4F; // 'O'
		data_[dataBytes2++] = 0x70; // 'p'
		data_[dataBytes2++] = 0x75; // 'u'
		data_[dataBytes2++] = 0x73; // 's'
		data_[dataBytes2++] = 0x48; // 'H'
		data_[dataBytes2++] = 0x65; // 'e'
		data_[dataBytes2++] = 0x61; // 'a'
		data_[dataBytes2++] = 0x64; // 'd'
		data_[dataBytes2++] = 0; // opus version
		data_[dataBytes2++] = channels;
		data_[dataBytes2++] = (lookahead_ * (/* samplesPerSec */ 48000) / samplesPerSec_) & 0xFF;
		data_[dataBytes2++] = ((lookahead_ * (/* samplesPerSec */ 48000) / samplesPerSec_) >> 8) & 0xFF;
		data_[dataBytes2++] = (/* samplesPerSec */ 48000) & 0xFF;
		data_[dataBytes2++] = ((/* samplesPerSec */ 48000) >> 8) & 0xFF;
		data_[dataBytes2++] = ((/* samplesPerSec */ 48000) >> 16) & 0xFF;
		data_[dataBytes2++] = ((/* samplesPerSec */ 48000) >> 24) & 0xFF;
		data_[dataBytes2++] = 0; // gain
		data_[dataBytes2++] = 0;
		data_[dataBytes2++] = 0; // channel mapping
		data_[dataBytes++] = dataBytes2 - 100; // 19
		write_(oggOpusData, data_,
							dataBytes,
							dataBytes2, /* headerTypeFlag */ 2,
										granulePosition_,
										bitstreamSerialNumber_,
										pageSequenceNumber_++);
		// -->
		// <!--
		dataBytes = 27;
		dataBytes2 = 100;
		data_[dataBytes2++] = 0x4F; // 'O'
		data_[dataBytes2++] = 0x70; // 'p'
		data_[dataBytes2++] = 0x75; // 'u'
		data_[dataBytes2++] = 0x73; // 's'
		data_[dataBytes2++] = 0x54; // 'T'
		data_[dataBytes2++] = 0x61; // 'a'
		data_[dataBytes2++] = 0x67; // 'g'
		data_[dataBytes2++] = 0x73; // 's'
		data_[dataBytes2++] = 0; // vendor length
		data_[dataBytes2++] = 0;
		data_[dataBytes2++] = 0;
		data_[dataBytes2++] = 0;
		data_[dataBytes2++] = 0; // comment list length
		data_[dataBytes2++] = 0;
		data_[dataBytes2++] = 0;
		data_[dataBytes2++] = 0;
		data_[dataBytes++] = dataBytes2 - 100; // 16
		write_(oggOpusData, data_,
							dataBytes,
							dataBytes2, /* headerTypeFlag */ 0,
										granulePosition_,
										bitstreamSerialNumber_,
										pageSequenceNumber_++);
		// -->
		// <!--
		dataBytes = 27;
		dataBytes2 = 100;
		// -->
		frames_ = 0;
		bytes_ = 0;
		samples_ = 0;
		while (true) {
			while (opusDataBytes > 0) {
				var frameDataBytes;
				if (bytesPerFrame_ !== 0) {
					frameDataBytes = bytesPerFrame_;
				} else {
					if (opusDataBytes < 2) {
						break;
					}
					frameDataBytes = opusData[opusDataIndex] | (opusData[opusDataIndex + 1] << 8);
					opusDataIndex += 2;
					opusDataBytes -= 2;
				}
				if (dataBytes - 27 >= 32 || dataBytes2 + frameDataBytes > data_.byteLength) {
					// <!--
					write_(oggOpusData, data_,
										dataBytes,
										dataBytes2, /* headerTypeFlag */ 0,
													granulePosition_,
													bitstreamSerialNumber_,
													pageSequenceNumber_++);
					// -->
					// <!--
					dataBytes = 27;
					dataBytes2 = 100;
					// -->
				}
				if (opusDataBytes < frameDataBytes) {
					break;
				}
				data_.set(opusData.subarray(opusDataIndex, opusDataIndex + frameDataBytes), dataBytes2);
				opusDataIndex += frameDataBytes;
				opusDataBytes -= frameDataBytes;
				dataBytes2 += frameDataBytes;
				frames_++;
				bytes_ += frameDataBytes;
				samples_ += (frames_ === 1) ? samplesPerFrame_ - lookahead_ : samplesPerFrame_;
				while (true) {
					if (frameDataBytes < 255) {
						data_[dataBytes++] = frameDataBytes;
						break;
					}
					data_[dataBytes++] = 255;
					frameDataBytes -= 255;
				}
				granulePosition_ += samplesPerFrame_ * (/* samplesPerSec */ 48000) / samplesPerSec_;
			}
			if (dataIndex >= data.length) {
				break;
			}
			opusData = data[dataIndex++];
			opusDataIndex = 0;
			opusDataBytes = opusData.length;
		}
		if (dataBytes - 27 > 0) {
			// <!--
			write_(oggOpusData, data_,
								dataBytes,
								dataBytes2, /* headerTypeFlag */ 0,
											granulePosition_,
											bitstreamSerialNumber_,
											pageSequenceNumber_++);
			// -->
		}
		if (false) {
			console.log("<OpusData> frames[" + frames_ + "]"
								+ " bytes[" + bytes_ + "]"
								+ " samples[" + samples_ + "(" + this.samples + ")] </OpusData>");
		}
		this.samples = samples_ * (/* samplesPerSec */ 48000) / samplesPerSec_ | 0;
		this.samplesPerSec = /* samplesPerSec */ 48000;
		this.mime = "audio/opus";
		return oggOpusData;
	}

	function convert__(opusData) {
		var opusDataIndex = 0;
		var opusDataBytes = opusData.length;
		if (opusDataBytes < 1) {
			return opusData;
		}
		opusDataIndex++;
		opusDataBytes--;
		// <!--
		var dataBytes = 0;
		data_[dataBytes++] = 0x70; // 'p'
		// -->
		if (samplesPerSec_ == 0) {
			if (opusDataBytes < 16) {
				return opusData;
			}
			samplesPerSec_ = opusData[opusDataIndex + 6] | (opusData[opusDataIndex + 7] << 8);
			var channels = opusData[opusDataIndex + 8];
			var complexity = opusData[opusDataIndex + 10];
			lookahead_ = opusData[opusDataIndex + 11];
			if (lookahead_ === 56) {
				lookahead_ = 312;
			}
			bytesPerFrame_ = opusData[opusDataIndex + 12];
			samplesPerFrame_ = opusData[opusDataIndex + 13] | (opusData[opusDataIndex + 14] << 8);
			opusDataIndex += 16;
			opusDataBytes -= 16;
			if (false) {
				console.log("<OpusHeader> samplesPerSec[" + samplesPerSec_ + "(" + this.samplesPerSec + ")]"
									  + " channels[" + channels + "]"
									  + " complexity[" + complexity + "]"
									  + " lookahead[" + lookahead_ + "]"
									  + " bytesPerFrame[" + bytesPerFrame_ + "]"
									  + " samplesPerFrame[" + samplesPerFrame_ + "] </OpusHeader>");
			}
			if (samplesPerSec_ != 48000) {
				return opusData;
			}
			// <!--
			var savedDataBytes = dataBytes;
			granulePosition_ = 0;
			bitstreamSerialNumber_ = /* as a random number */ 7584;
			pageSequenceNumber_ = 0;
			data_[dataBytes++] = 0x4F; // 'O'
			data_[dataBytes++] = 0x67; // 'g'
			data_[dataBytes++] = 0x67; // 'g'
			data_[dataBytes++] = 0x53; // 'S'
			data_[dataBytes++] = 0; // stream structure version
			data_[dataBytes++] = 2; // header type flag
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = bitstreamSerialNumber_ & 0xFF;
			data_[dataBytes++] = (bitstreamSerialNumber_ >> 8) & 0xFF;
			data_[dataBytes++] = (bitstreamSerialNumber_ >> 16) & 0xFF;
			data_[dataBytes++] = (bitstreamSerialNumber_ >> 24) & 0xFF;
			data_[dataBytes++] = pageSequenceNumber_ & 0xFF;
			data_[dataBytes++] = (pageSequenceNumber_ >> 8) & 0xFF;
			data_[dataBytes++] = (pageSequenceNumber_ >> 16) & 0xFF;
			data_[dataBytes++] = (pageSequenceNumber_ >> 24) & 0xFF;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 1;
			data_[dataBytes++] = 19;
			data_[dataBytes++] = 0x4F; // 'O'
			data_[dataBytes++] = 0x70; // 'p'
			data_[dataBytes++] = 0x75; // 'u'
			data_[dataBytes++] = 0x73; // 's'
			data_[dataBytes++] = 0x48; // 'H'
			data_[dataBytes++] = 0x65; // 'e'
			data_[dataBytes++] = 0x61; // 'a'
			data_[dataBytes++] = 0x64; // 'd'
			data_[dataBytes++] = 0; // opus version
			data_[dataBytes++] = channels;
			data_[dataBytes++] = (lookahead_ * (/* samplesPerSec */ 48000) / samplesPerSec_) & 0xFF;
			data_[dataBytes++] = ((lookahead_ * (/* samplesPerSec */ 48000) / samplesPerSec_) >> 8) & 0xFF;
			data_[dataBytes++] = (/* samplesPerSec */ 48000) & 0xFF;
			data_[dataBytes++] = ((/* samplesPerSec */ 48000) >> 8) & 0xFF;
			data_[dataBytes++] = ((/* samplesPerSec */ 48000) >> 16) & 0xFF;
			data_[dataBytes++] = ((/* samplesPerSec */ 48000) >> 24) & 0xFF;
			data_[dataBytes++] = 0; // gain
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0; // channel mapping
			var crc = crc_(0, data_, savedDataBytes, dataBytes);
			data_[savedDataBytes + 22] = crc & 0xFF;
			data_[savedDataBytes + 23] = (crc >> 8) & 0xFF;
			data_[savedDataBytes + 24] = (crc >> 16) & 0xFF;
			data_[savedDataBytes + 25] = (crc >> 24) & 0xFF;
			// -->
			// <!--
			savedDataBytes = dataBytes;
			pageSequenceNumber_++;
			data_[dataBytes++] = 0x4F; // 'O'
			data_[dataBytes++] = 0x67; // 'g'
			data_[dataBytes++] = 0x67; // 'g'
			data_[dataBytes++] = 0x53; // 'S'
			data_[dataBytes++] = 0; // stream structure version
			data_[dataBytes++] = 0; // header type flag
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = bitstreamSerialNumber_ & 0xFF;
			data_[dataBytes++] = (bitstreamSerialNumber_ >> 8) & 0xFF;
			data_[dataBytes++] = (bitstreamSerialNumber_ >> 16) & 0xFF;
			data_[dataBytes++] = (bitstreamSerialNumber_ >> 24) & 0xFF;
			data_[dataBytes++] = pageSequenceNumber_ & 0xFF;
			data_[dataBytes++] = (pageSequenceNumber_ >> 8) & 0xFF;
			data_[dataBytes++] = (pageSequenceNumber_ >> 16) & 0xFF;
			data_[dataBytes++] = (pageSequenceNumber_ >> 24) & 0xFF;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 1;
			data_[dataBytes++] = 16;
			data_[dataBytes++] = 0x4F; // 'O'
			data_[dataBytes++] = 0x70; // 'p'
			data_[dataBytes++] = 0x75; // 'u'
			data_[dataBytes++] = 0x73; // 's'
			data_[dataBytes++] = 0x54; // 'T'
			data_[dataBytes++] = 0x61; // 'a'
			data_[dataBytes++] = 0x67; // 'g'
			data_[dataBytes++] = 0x73; // 's'
			data_[dataBytes++] = 0; // vendor length
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0; // comment list length
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			data_[dataBytes++] = 0;
			var crc = crc_(0, data_, savedDataBytes, dataBytes);
			data_[savedDataBytes + 22] = crc & 0xFF;
			data_[savedDataBytes + 23] = (crc >> 8) & 0xFF;
			data_[savedDataBytes + 24] = (crc >> 16) & 0xFF;
			data_[savedDataBytes + 25] = (crc >> 24) & 0xFF;
			// -->
			frames_ = 0;
			bytes_ = 0;
			samples_ = 0;
		} else {
			if (samplesPerSec_ != 48000) {
				return opusData;
			}
		}
		// <!--
		var savedDataBytes = dataBytes;
		pageSequenceNumber_++;
		data_[dataBytes++] = 0x4F; // 'O'
		data_[dataBytes++] = 0x67; // 'g'
		data_[dataBytes++] = 0x67; // 'g'
		data_[dataBytes++] = 0x53; // 'S'
		data_[dataBytes++] = 0; // stream structure version
		data_[dataBytes++] = 0; // header type flag
		dataBytes++;
		dataBytes++;
		dataBytes++;
		dataBytes++;
		data_[dataBytes++] = 0;
		data_[dataBytes++] = 0;
		data_[dataBytes++] = 0;
		data_[dataBytes++] = 0;
		data_[dataBytes++] = bitstreamSerialNumber_ & 0xFF;
		data_[dataBytes++] = (bitstreamSerialNumber_ >> 8) & 0xFF;
		data_[dataBytes++] = (bitstreamSerialNumber_ >> 16) & 0xFF;
		data_[dataBytes++] = (bitstreamSerialNumber_ >> 24) & 0xFF;
		data_[dataBytes++] = pageSequenceNumber_ & 0xFF;
		data_[dataBytes++] = (pageSequenceNumber_ >> 8) & 0xFF;
		data_[dataBytes++] = (pageSequenceNumber_ >> 16) & 0xFF;
		data_[dataBytes++] = (pageSequenceNumber_ >> 24) & 0xFF;
		data_[dataBytes++] = 0;
		data_[dataBytes++] = 0;
		data_[dataBytes++] = 0;
		data_[dataBytes++] = 0;
		dataBytes++;
		var savedOpusDataIndex = opusDataIndex;
		var savedOpusDataBytes = opusDataBytes;
		while (opusDataBytes > 0) {
			var frameDataBytes;
			if (bytesPerFrame_ !== 0) {
				frameDataBytes = bytesPerFrame_;
			} else {
				if (opusDataBytes < 2) {
					break;
				}
				frameDataBytes = opusData[opusDataIndex] | (opusData[opusDataIndex + 1] << 8);
				opusDataIndex += 2;
				opusDataBytes -= 2;
			}
			if (opusDataBytes < frameDataBytes) {
				break;
			}
			opusDataIndex += frameDataBytes;
			opusDataBytes -= frameDataBytes;
			while (true) {
				if (frameDataBytes < 255) {
					data_[dataBytes++] = frameDataBytes;
					break;
				}
				data_[dataBytes++] = 255;
				frameDataBytes -= 255;
			}
			granulePosition_ += samplesPerFrame_ * (/* samplesPerSec */ 48000) / samplesPerSec_;
		}
		opusDataIndex = savedOpusDataIndex;
		opusDataBytes = savedOpusDataBytes;
		data_[savedDataBytes + 6] = granulePosition_ & 0xFF;
		data_[savedDataBytes + 7] = (granulePosition_ >> 8) & 0xFF;
		data_[savedDataBytes + 8] = (granulePosition_ >> 16) & 0xFF;
		data_[savedDataBytes + 9] = (granulePosition_ >> 24) & 0xFF;
		data_[savedDataBytes + 26] = dataBytes - (savedDataBytes + 27);
		while (opusDataBytes > 0) {
			var frameDataBytes;
			if (bytesPerFrame_ !== 0) {
				frameDataBytes = bytesPerFrame_;
			} else {
				if (opusDataBytes < 2) {
					break;
				}
				frameDataBytes = opusData[opusDataIndex] | (opusData[opusDataIndex + 1] << 8);
				opusDataIndex += 2;
				opusDataBytes -= 2;
			}
			if (opusDataBytes < frameDataBytes) {
				break;
			}
			data_.set(opusData.subarray(opusDataIndex, opusDataIndex + frameDataBytes), dataBytes);
			opusDataIndex += frameDataBytes;
			opusDataBytes -= frameDataBytes;
			dataBytes += frameDataBytes;
			frames_++;
			bytes_ += frameDataBytes;
			samples_ += (frames_ === 1) ? samplesPerFrame_ - lookahead_ : samplesPerFrame_;
		}
		var crc = crc_(0, data_, savedDataBytes, dataBytes);
		data_[savedDataBytes + 22] = crc & 0xFF;
		data_[savedDataBytes + 23] = (crc >> 8) & 0xFF;
		data_[savedDataBytes + 24] = (crc >> 16) & 0xFF;
		data_[savedDataBytes + 25] = (crc >> 24) & 0xFF;
		// -->
		return data_.subarray(0, dataBytes);
	}

	function write_(oggOpusData, data_,
								 dataBytes,
								 dataBytes2, headerTypeFlag,
											 granulePosition_,
											 bitstreamSerialNumber_,
											 pageSequenceNumber_) {
		data_[ 0] = 0x4F; // 'O'
		data_[ 1] = 0x67; // 'g'
		data_[ 2] = 0x67; // 'g'
		data_[ 3] = 0x53; // 'S'
		data_[ 4] = 0; // stream structure version
		data_[ 5] = headerTypeFlag;
		data_[ 6] = granulePosition_ & 0xFF;
		data_[ 7] = (granulePosition_ >> 8) & 0xFF;
		data_[ 8] = (granulePosition_ >> 16) & 0xFF;
		data_[ 9] = (granulePosition_ >> 24) & 0xFF;
		data_[10] = 0;
		data_[11] = 0;
		data_[12] = 0;
		data_[13] = 0;
		data_[14] = bitstreamSerialNumber_ & 0xFF;
		data_[15] = (bitstreamSerialNumber_ >> 8) & 0xFF;
		data_[16] = (bitstreamSerialNumber_ >> 16) & 0xFF;
		data_[17] = (bitstreamSerialNumber_ >> 24) & 0xFF;
		data_[18] = pageSequenceNumber_ & 0xFF;
		data_[19] = (pageSequenceNumber_ >> 8) & 0xFF;
		data_[20] = (pageSequenceNumber_ >> 16) & 0xFF;
		data_[21] = (pageSequenceNumber_ >> 24) & 0xFF;
		data_[22] = 0;
		data_[23] = 0;
		data_[24] = 0;
		data_[25] = 0;
		data_[26] = dataBytes - 27;
		var crc = crc_(crc_(0, data_, 0, dataBytes), data_, 100, dataBytes2);
		data_[22] = crc & 0xFF;
		data_[23] = (crc >> 8) & 0xFF;
		data_[24] = (crc >> 16) & 0xFF;
		data_[25] = (crc >> 24) & 0xFF;
		oggOpusData.push(new Uint8Array(data_.subarray(0, dataBytes))); // deep copy
		oggOpusData.push(new Uint8Array(data_.subarray(100, dataBytes2))); // deep copy
	}

	var crc_lookup_ = new Uint32Array(256);
	crc_lookup_[  0] = 0x00000000; crc_lookup_[  1] = 0x04c11db7; crc_lookup_[  2] = 0x09823b6e; crc_lookup_[  3] = 0x0d4326d9;
	crc_lookup_[  4] = 0x130476dc; crc_lookup_[  5] = 0x17c56b6b; crc_lookup_[  6] = 0x1a864db2; crc_lookup_[  7] = 0x1e475005;
	crc_lookup_[  8] = 0x2608edb8; crc_lookup_[  9] = 0x22c9f00f; crc_lookup_[ 10] = 0x2f8ad6d6; crc_lookup_[ 11] = 0x2b4bcb61;
	crc_lookup_[ 12] = 0x350c9b64; crc_lookup_[ 13] = 0x31cd86d3; crc_lookup_[ 14] = 0x3c8ea00a; crc_lookup_[ 15] = 0x384fbdbd;
	crc_lookup_[ 16] = 0x4c11db70; crc_lookup_[ 17] = 0x48d0c6c7; crc_lookup_[ 18] = 0x4593e01e; crc_lookup_[ 19] = 0x4152fda9;
	crc_lookup_[ 20] = 0x5f15adac; crc_lookup_[ 21] = 0x5bd4b01b; crc_lookup_[ 22] = 0x569796c2; crc_lookup_[ 23] = 0x52568b75;
	crc_lookup_[ 24] = 0x6a1936c8; crc_lookup_[ 25] = 0x6ed82b7f; crc_lookup_[ 26] = 0x639b0da6; crc_lookup_[ 27] = 0x675a1011;
	crc_lookup_[ 28] = 0x791d4014; crc_lookup_[ 29] = 0x7ddc5da3; crc_lookup_[ 30] = 0x709f7b7a; crc_lookup_[ 31] = 0x745e66cd;
	crc_lookup_[ 32] = 0x9823b6e0; crc_lookup_[ 33] = 0x9ce2ab57; crc_lookup_[ 34] = 0x91a18d8e; crc_lookup_[ 35] = 0x95609039;
	crc_lookup_[ 36] = 0x8b27c03c; crc_lookup_[ 37] = 0x8fe6dd8b; crc_lookup_[ 38] = 0x82a5fb52; crc_lookup_[ 39] = 0x8664e6e5;
	crc_lookup_[ 40] = 0xbe2b5b58; crc_lookup_[ 41] = 0xbaea46ef; crc_lookup_[ 42] = 0xb7a96036; crc_lookup_[ 43] = 0xb3687d81;
	crc_lookup_[ 44] = 0xad2f2d84; crc_lookup_[ 45] = 0xa9ee3033; crc_lookup_[ 46] = 0xa4ad16ea; crc_lookup_[ 47] = 0xa06c0b5d;
	crc_lookup_[ 48] = 0xd4326d90; crc_lookup_[ 49] = 0xd0f37027; crc_lookup_[ 50] = 0xddb056fe; crc_lookup_[ 51] = 0xd9714b49;
	crc_lookup_[ 52] = 0xc7361b4c; crc_lookup_[ 53] = 0xc3f706fb; crc_lookup_[ 54] = 0xceb42022; crc_lookup_[ 55] = 0xca753d95;
	crc_lookup_[ 56] = 0xf23a8028; crc_lookup_[ 57] = 0xf6fb9d9f; crc_lookup_[ 58] = 0xfbb8bb46; crc_lookup_[ 59] = 0xff79a6f1;
	crc_lookup_[ 60] = 0xe13ef6f4; crc_lookup_[ 61] = 0xe5ffeb43; crc_lookup_[ 62] = 0xe8bccd9a; crc_lookup_[ 63] = 0xec7dd02d;
	crc_lookup_[ 64] = 0x34867077; crc_lookup_[ 65] = 0x30476dc0; crc_lookup_[ 66] = 0x3d044b19; crc_lookup_[ 67] = 0x39c556ae;
	crc_lookup_[ 68] = 0x278206ab; crc_lookup_[ 69] = 0x23431b1c; crc_lookup_[ 70] = 0x2e003dc5; crc_lookup_[ 71] = 0x2ac12072;
	crc_lookup_[ 72] = 0x128e9dcf; crc_lookup_[ 73] = 0x164f8078; crc_lookup_[ 74] = 0x1b0ca6a1; crc_lookup_[ 75] = 0x1fcdbb16;
	crc_lookup_[ 76] = 0x018aeb13; crc_lookup_[ 77] = 0x054bf6a4; crc_lookup_[ 78] = 0x0808d07d; crc_lookup_[ 79] = 0x0cc9cdca;
	crc_lookup_[ 80] = 0x7897ab07; crc_lookup_[ 81] = 0x7c56b6b0; crc_lookup_[ 82] = 0x71159069; crc_lookup_[ 83] = 0x75d48dde;
	crc_lookup_[ 84] = 0x6b93dddb; crc_lookup_[ 85] = 0x6f52c06c; crc_lookup_[ 86] = 0x6211e6b5; crc_lookup_[ 87] = 0x66d0fb02;
	crc_lookup_[ 88] = 0x5e9f46bf; crc_lookup_[ 89] = 0x5a5e5b08; crc_lookup_[ 90] = 0x571d7dd1; crc_lookup_[ 91] = 0x53dc6066;
	crc_lookup_[ 92] = 0x4d9b3063; crc_lookup_[ 93] = 0x495a2dd4; crc_lookup_[ 94] = 0x44190b0d; crc_lookup_[ 95] = 0x40d816ba;
	crc_lookup_[ 96] = 0xaca5c697; crc_lookup_[ 97] = 0xa864db20; crc_lookup_[ 98] = 0xa527fdf9; crc_lookup_[ 99] = 0xa1e6e04e;
	crc_lookup_[100] = 0xbfa1b04b; crc_lookup_[101] = 0xbb60adfc; crc_lookup_[102] = 0xb6238b25; crc_lookup_[103] = 0xb2e29692;
	crc_lookup_[104] = 0x8aad2b2f; crc_lookup_[105] = 0x8e6c3698; crc_lookup_[106] = 0x832f1041; crc_lookup_[107] = 0x87ee0df6;
	crc_lookup_[108] = 0x99a95df3; crc_lookup_[109] = 0x9d684044; crc_lookup_[110] = 0x902b669d; crc_lookup_[111] = 0x94ea7b2a;
	crc_lookup_[112] = 0xe0b41de7; crc_lookup_[113] = 0xe4750050; crc_lookup_[114] = 0xe9362689; crc_lookup_[115] = 0xedf73b3e;
	crc_lookup_[116] = 0xf3b06b3b; crc_lookup_[117] = 0xf771768c; crc_lookup_[118] = 0xfa325055; crc_lookup_[119] = 0xfef34de2;
	crc_lookup_[120] = 0xc6bcf05f; crc_lookup_[121] = 0xc27dede8; crc_lookup_[122] = 0xcf3ecb31; crc_lookup_[123] = 0xcbffd686;
	crc_lookup_[124] = 0xd5b88683; crc_lookup_[125] = 0xd1799b34; crc_lookup_[126] = 0xdc3abded; crc_lookup_[127] = 0xd8fba05a;
	crc_lookup_[128] = 0x690ce0ee; crc_lookup_[129] = 0x6dcdfd59; crc_lookup_[130] = 0x608edb80; crc_lookup_[131] = 0x644fc637;
	crc_lookup_[132] = 0x7a089632; crc_lookup_[133] = 0x7ec98b85; crc_lookup_[134] = 0x738aad5c; crc_lookup_[135] = 0x774bb0eb;
	crc_lookup_[136] = 0x4f040d56; crc_lookup_[137] = 0x4bc510e1; crc_lookup_[138] = 0x46863638; crc_lookup_[139] = 0x42472b8f;
	crc_lookup_[140] = 0x5c007b8a; crc_lookup_[141] = 0x58c1663d; crc_lookup_[142] = 0x558240e4; crc_lookup_[143] = 0x51435d53;
	crc_lookup_[144] = 0x251d3b9e; crc_lookup_[145] = 0x21dc2629; crc_lookup_[146] = 0x2c9f00f0; crc_lookup_[147] = 0x285e1d47;
	crc_lookup_[148] = 0x36194d42; crc_lookup_[149] = 0x32d850f5; crc_lookup_[150] = 0x3f9b762c; crc_lookup_[151] = 0x3b5a6b9b;
	crc_lookup_[152] = 0x0315d626; crc_lookup_[153] = 0x07d4cb91; crc_lookup_[154] = 0x0a97ed48; crc_lookup_[155] = 0x0e56f0ff;
	crc_lookup_[156] = 0x1011a0fa; crc_lookup_[157] = 0x14d0bd4d; crc_lookup_[158] = 0x19939b94; crc_lookup_[159] = 0x1d528623;
	crc_lookup_[160] = 0xf12f560e; crc_lookup_[161] = 0xf5ee4bb9; crc_lookup_[162] = 0xf8ad6d60; crc_lookup_[163] = 0xfc6c70d7;
	crc_lookup_[164] = 0xe22b20d2; crc_lookup_[165] = 0xe6ea3d65; crc_lookup_[166] = 0xeba91bbc; crc_lookup_[167] = 0xef68060b;
	crc_lookup_[168] = 0xd727bbb6; crc_lookup_[169] = 0xd3e6a601; crc_lookup_[170] = 0xdea580d8; crc_lookup_[171] = 0xda649d6f;
	crc_lookup_[172] = 0xc423cd6a; crc_lookup_[173] = 0xc0e2d0dd; crc_lookup_[174] = 0xcda1f604; crc_lookup_[175] = 0xc960ebb3;
	crc_lookup_[176] = 0xbd3e8d7e; crc_lookup_[177] = 0xb9ff90c9; crc_lookup_[178] = 0xb4bcb610; crc_lookup_[179] = 0xb07daba7;
	crc_lookup_[180] = 0xae3afba2; crc_lookup_[181] = 0xaafbe615; crc_lookup_[182] = 0xa7b8c0cc; crc_lookup_[183] = 0xa379dd7b;
	crc_lookup_[184] = 0x9b3660c6; crc_lookup_[185] = 0x9ff77d71; crc_lookup_[186] = 0x92b45ba8; crc_lookup_[187] = 0x9675461f;
	crc_lookup_[188] = 0x8832161a; crc_lookup_[189] = 0x8cf30bad; crc_lookup_[190] = 0x81b02d74; crc_lookup_[191] = 0x857130c3;
	crc_lookup_[192] = 0x5d8a9099; crc_lookup_[193] = 0x594b8d2e; crc_lookup_[194] = 0x5408abf7; crc_lookup_[195] = 0x50c9b640;
	crc_lookup_[196] = 0x4e8ee645; crc_lookup_[197] = 0x4a4ffbf2; crc_lookup_[198] = 0x470cdd2b; crc_lookup_[199] = 0x43cdc09c;
	crc_lookup_[200] = 0x7b827d21; crc_lookup_[201] = 0x7f436096; crc_lookup_[202] = 0x7200464f; crc_lookup_[203] = 0x76c15bf8;
	crc_lookup_[204] = 0x68860bfd; crc_lookup_[205] = 0x6c47164a; crc_lookup_[206] = 0x61043093; crc_lookup_[207] = 0x65c52d24;
	crc_lookup_[208] = 0x119b4be9; crc_lookup_[209] = 0x155a565e; crc_lookup_[210] = 0x18197087; crc_lookup_[211] = 0x1cd86d30;
	crc_lookup_[212] = 0x029f3d35; crc_lookup_[213] = 0x065e2082; crc_lookup_[214] = 0x0b1d065b; crc_lookup_[215] = 0x0fdc1bec;
	crc_lookup_[216] = 0x3793a651; crc_lookup_[217] = 0x3352bbe6; crc_lookup_[218] = 0x3e119d3f; crc_lookup_[219] = 0x3ad08088;
	crc_lookup_[220] = 0x2497d08d; crc_lookup_[221] = 0x2056cd3a; crc_lookup_[222] = 0x2d15ebe3; crc_lookup_[223] = 0x29d4f654;
	crc_lookup_[224] = 0xc5a92679; crc_lookup_[225] = 0xc1683bce; crc_lookup_[226] = 0xcc2b1d17; crc_lookup_[227] = 0xc8ea00a0;
	crc_lookup_[228] = 0xd6ad50a5; crc_lookup_[229] = 0xd26c4d12; crc_lookup_[230] = 0xdf2f6bcb; crc_lookup_[231] = 0xdbee767c;
	crc_lookup_[232] = 0xe3a1cbc1; crc_lookup_[233] = 0xe760d676; crc_lookup_[234] = 0xea23f0af; crc_lookup_[235] = 0xeee2ed18;
	crc_lookup_[236] = 0xf0a5bd1d; crc_lookup_[237] = 0xf464a0aa; crc_lookup_[238] = 0xf9278673; crc_lookup_[239] = 0xfde69bc4;
	crc_lookup_[240] = 0x89b8fd09; crc_lookup_[241] = 0x8d79e0be; crc_lookup_[242] = 0x803ac667; crc_lookup_[243] = 0x84fbdbd0;
	crc_lookup_[244] = 0x9abc8bd5; crc_lookup_[245] = 0x9e7d9662; crc_lookup_[246] = 0x933eb0bb; crc_lookup_[247] = 0x97ffad0c;
	crc_lookup_[248] = 0xafb010b1; crc_lookup_[249] = 0xab710d06; crc_lookup_[250] = 0xa6322bdf; crc_lookup_[251] = 0xa2f33668;
	crc_lookup_[252] = 0xbcb4666d; crc_lookup_[253] = 0xb8757bda; crc_lookup_[254] = 0xb5365d03; crc_lookup_[255] = 0xb1f740b4;

	function crc_(crc, data, dataStartIndex, dataEndIndex) {
		for (var i = dataStartIndex; i < dataEndIndex; i++) {
			crc = (crc << 8) ^ crc_lookup_[((crc >> 24) ^ data[i]) & 0xFF];
		}
		return crc;
	}

	if (window.Encoder && window.Encoder.OpusEncoder) {
		window.Encoder.OpusEncoder.convert = convert_;
		if (window.Recorder) {
			window.Recorder.sampleRate = 48000;
		}
	}
})();
