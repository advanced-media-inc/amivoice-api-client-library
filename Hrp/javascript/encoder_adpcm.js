window.Encoder = window.Encoder || {};

window.Encoder.ADPCMEncoder = window.Encoder.ADPCMEncoder || {version: "ADPCMEncoder/1.0.11"};

window.Encoder.createADPCMEncoder = function (samplesPerSec) {
	var samplesPerSec_ = samplesPerSec;
	var header_ = 0;
	var last_ = 0;
	var index_ = 0;

	var stepSizeTable_ = [
		7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
		19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
		50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
		130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
		337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
		876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
		2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
		5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
		15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
	];

	var stepAdjustTable_ = [
		-1, -1, -1, -1, 2, 4, 6, 8
	];

	function linear2ima_(pcm) {
		var step_size = stepSizeTable_[index_];
		var diff = pcm - last_;
		var ima = 0x00;
		if (diff < 0) {
			ima = 0x08;
			diff = -diff;
		}
		var vpdiff = 0;
		if (diff >= step_size) {
			ima |= 0x04;
			diff -= step_size;
			vpdiff += step_size;
		}
		step_size >>= 1;
		if (diff >= step_size) {
			ima |= 0x02;
			diff -= step_size;
			vpdiff += step_size;
		}
		step_size >>= 1;
		if (diff >= step_size) {
			ima |= 0x01;
			vpdiff += step_size;
		}
		step_size >>= 1;
		vpdiff += step_size;
		if ((ima & 0x08) != 0) {
			last_ -= vpdiff;
		} else {
			last_ += vpdiff;
		}
		if (last_ > 32767) {
			last_ = 32767;
		} else
		if (last_ < -32768) {
			last_ = -32768;
		}
		index_ += stepAdjustTable_[ima & 0x07];
		if (index_ < 0) {
			index_ = 0;
		} else
		if (index_ > 88) {
			index_ = 88;
		}
		return ima;
	}

	function encode_(data) {
		var readingDataByteCount = data.byteLength;
		var readDataByteCount = 0;
		var writtenDataByteCount = 0;
		var dataView = new DataView(data.buffer, data.byteOffset, data.byteLength);
		while (readingDataByteCount >= 4) {
			var pcm1 = dataView.getInt16(readDataByteCount    , true);
			var pcm2 = dataView.getInt16(readDataByteCount + 2, true);
			readingDataByteCount -= 4;
			readDataByteCount += 4;
			var ima1 = linear2ima_(pcm1);
			var ima2 = linear2ima_(pcm2);
			data[writtenDataByteCount++] = (ima1 << 4) | ima2;
		}
		if (writtenDataByteCount == 0) {
			return null;
		}
		this.samples += readDataByteCount / 2;
		if (!header_) {
			data = new Uint8Array(data.buffer, data.byteOffset - 16, 16 + writtenDataByteCount);
			data[ 0] = 0x23; // '#'
			data[ 1] = 0x21; // '!'
			data[ 2] = 0x41; // 'A'
			data[ 3] = 0x44; // 'D'
			data[ 4] = 0x50; // 'P'
			data[ 5] = 0x0A; // '\n'
			data[ 6] = samplesPerSec_ & 0xFF;
			data[ 7] = (samplesPerSec_ >> 8) & 0xFF;
			data[ 8] = 1;
			data[ 9] = 2; // adpcm_type: DVI/IMA ADPCM
			data[10] = 0;
			data[11] = 0;
			data[12] = 1;
			data[13] = 2;
			data[14] = 0;
			data[15] = 0;
			header_ = 1;
		} else {
			data = new Uint8Array(data.buffer, data.byteOffset, writtenDataByteCount);
		}
		return data;
	}

	function destroy_() {
	}

	return {
		...window.Encoder.ADPCMEncoder,
		encode: encode_,
		destroy: destroy_,
		samples: 0,
		samplesPerSec: samplesPerSec_,
		bytesPerSec: samplesPerSec_ * 2 / 4,
		framesPerSec: 0,
		mime: "audio/x-adp",
		extension: ".adp"
	};
};

if (window.Recorder) {
	window.Recorder.adpcmPacking = true;
}
