var Recorder = function () {
	// window.AudioContext()
	window.AudioContext = window.AudioContext || window.webkitAudioContext || window.mozAudioContext || window.msAudioContext;
	// navigator.getUserMedia()
	navigator.getUserMedia = navigator.getUserMedia || navigator.webkitGetUserMedia || navigator.mozGetUserMedia || navigator.msGetUserMedia;
	// navigator.mediaDevices.getUserMedia()
	navigator.mediaDevices = navigator.mediaDevices || ((navigator.getUserMedia) ? {
		getUserMedia: function (c) {
			return new Promise(
				function (y, n) {
					navigator.getUserMedia(c, y, n);
				}
			);
		}
	} : null);

	// public オブジェクト
	var recorder_ = {
		// public プロパティ
		version: "Recorder/1.0.12",
		recordingSource: 1,
		recordingSourceElement: undefined,
		sampleRate: 16000,
		sampleRateElement: undefined,
		maxRecordingTime: 60000,
		maxRecordingTimeElement: undefined,
		downSampling: false,
		downSamplingElement: undefined,
		adpcmPacking: false,
		adpcmPackingElement: undefined,
		opusPacking: false,
		opusPackingElement: undefined,
		// public メソッド
		resume: resume_,
		pause: pause_,
		isActive: isActive_,
		getConverter: getConverter_,
		// イベントハンドラ
		resumeStarted: undefined,
		resumeEnded: undefined,
		recorded: undefined,
		pauseStarted: undefined,
		pauseEnded: undefined,
		TRACE: undefined
	};

	// 録音関連
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//  ┌────┐
	//  │0       │←───────────────────────┐┐┐
	//  └┬───┘                                                │││
	//    │resume()                                                │││
	//    │ - getUserMedia()                                       │││
	//    │                                                        │││
	//    ↓resumeStarted                                           │││
	//  ┌────┐                                                │││
	//  │1       │                                                │││
	//  └┬┬┬─┘                                                │││
	//    │││getUserMedia() 失敗                                 │││
	//    │││                                                    │││
	//    │││pauseEnded                                          │││
	//    ││└──────────────────────────┘││
	//    ││                                                        ││
	//    ││getUserMedia() 成功 && state_==3                        ││
	//    ││ - audioStream.getTracks().forEach(track=>track.stop()) ││
	//    ││                                                        ││
	//    ││pauseEnded                                              ││
	//    │└────────────────────────────┘│
	//    │                                                            │
	//    │getUserMedia() 成功 && state_==1                            │
	//    │ - audioProvider_=audioContext_.createMediaStreamSource     │
	//    │                                             (audioStream_) │
	//    │ - audioProvider_.connect(audioProcessor_)                  │
	//    │ - audioProcessor_.connect(audioContext_.destination)       │
	//    │                                                            │
	//    ↓resumeEnded                                                 │
	//  ┌────┐                                                    │
	//  │2 録音中│←────────────────────────┐│
	//  └┬┬──┘                                                  ││
	//    ││audioProcessor_.onaudioprocess                          ││
	//    │└────────────────────────────┘│
	//    │                                                            │
	//    │pause()                                                     │
	//    │                                                            │
	//    ↓pauseStarted                                                │
	//  ┌────┐                                                    │
	//  │3       │                                                    │
	//  └┬───┘                                                    │
	//    │audioProcessor_.onaudioprocess                              │
	//    │ - audioStream_.getTracks().forEach(track=>track.stop())    │
	//    │ - audioProvider_.disconnect()                              │
	//    │ - audioProcessor_.disconnect()                             │
	//    ↓                                                            │
	//  ┌────┐                                                    │
	//  │4       │                                                    │
	//  └┬───┘                                                    │
	//    │pauseEnded                                                  │
	//    └──────────────────────────────┘
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	var state_ = -1;
	var audioContext_;
	var audioProcessor_;
	var audioProcessor_onaudioprocess_;
	var audioProcessor_onaudioprocess_recorded_;
	var audioProcessor_onaudioprocess_downSampling_;
	var audioProcessor_onaudioprocess_downSampling_recorded_;
	var audioStream_;
	var audioProvider_;
	var audioSamplesPerSec_;
	var audioDecimationFactor_;
	var temporaryAudioData_;
	var temporaryAudioDataSamples_;
	var coefData_;
	var pcmData_;
	var waveData_;
	var waveFile_;
	var encoder_;
	var converter_;
	var reason_;
	var maxRecordingTimeTimerId_;

	// 各種変数の初期化
	async function initialize_() {
		// 録音関係の各種変数の初期化
		audioContext_ = new AudioContext({sampleRate: recorder_.sampleRate});
		recorder_.sampleRate = audioContext_.sampleRate; // public プロパティを通して外部から参照可能にしておく...
		await audioContext_.audioWorklet.addModule(URL.createObjectURL(new Blob([
			"registerProcessor('audioWorkletProcessor', class extends AudioWorkletProcessor {",
			"  constructor() {",
			"    super()",
			"  }",
			"  process(inputs, outputs, parameters) {",
			"    if (inputs.length > 0 && inputs[0].length > 0) {",
			"      if (inputs[0].length === 2) {",
			"        for (var j = 0; j < inputs[0][0].length; j++) {",
			"          inputs[0][0][j] = (inputs[0][0][j] + inputs[0][1][j]) / 2",
			"        }",
			"      }",
			"      this.port.postMessage(inputs[0][0], [inputs[0][0].buffer])",
			"    }",
			"    return true",
			"  }",
			"})"
		], {type: 'application/javascript'})));
		audioProcessor_ = new AudioWorkletNode(audioContext_, 'audioWorkletProcessor');
		audioProcessor_.bufferSize = 128;
		audioProcessor_onaudioprocess_ = function (event) {
			// <!-- for AudioWorklet
			if (state_ === 0) {
				return;
			}
			// -->
			var audioData = event.data;
			var pcmDataBytes = audioData.length * 2;
			var pcmData = new Uint8Array(new ArrayBuffer(16 + pcmDataBytes), 16, pcmDataBytes);
			var pcmDataIndex = 0;
			for (var audioDataIndex = 0; audioDataIndex < audioData.length; audioDataIndex++) {
				var pcm = audioData[audioDataIndex] * 32768 | 0; // 小数 (0.0～1.0) を 整数 (-32768～32767) に変換...
				if (pcm > 32767) {
					pcm = 32767;
				} else
				if (pcm < -32768) {
					pcm = -32768;
				}
				pcmData[pcmDataIndex++] = (pcm     ) & 0xFF;
				pcmData[pcmDataIndex++] = (pcm >> 8) & 0xFF;
			}
			if (!encoder_) {
				waveData_.push(pcmData);
			} else {
				pcmData = encoder_.encode(pcmData);
				if (pcmData) {
					waveData_.push(pcmData);
				}
			}
			if (state_ === 3) {
				state_ = 4;
				audioStream_.stopTracks();
				audioStream_ = undefined;
				audioProvider_.disconnect();
				audioProvider_ = undefined;
				audioProcessor_.disconnect();
				if (recorder_.TRACE) recorder_.TRACE("INFO: stopped recording");
			}
		};
		audioProcessor_onaudioprocess_recorded_ = function (event) {
			// <!-- for AudioWorklet
			if (state_ === 0) {
				return;
			}
			// -->
			var audioData = event.data;
			var pcmDataOffset = (encoder_) ? 1 + 16 : 1;
			var pcmDataIndex = pcmDataOffset;
			for (var audioDataIndex = 0; audioDataIndex < audioData.length; audioDataIndex++) {
				var pcm = audioData[audioDataIndex] * 32768 | 0; // 小数 (0.0～1.0) を 整数 (-32768～32767) に変換...
				if (pcm > 32767) {
					pcm = 32767;
				} else
				if (pcm < -32768) {
					pcm = -32768;
				}
				pcmData_[pcmDataIndex++] = (pcm     ) & 0xFF;
				pcmData_[pcmDataIndex++] = (pcm >> 8) & 0xFF;
			}
			var pcmData = pcmData_.subarray(pcmDataOffset, pcmDataIndex);
			if (!encoder_) {
				recorder_.recorded(pcmData);
			} else {
				pcmData = encoder_.encode(pcmData);
				if (pcmData) {
					recorder_.recorded(pcmData);
				}
			}
			if (state_ === 3) {
				state_ = 4;
				audioStream_.stopTracks();
				audioStream_ = undefined;
				audioProvider_.disconnect();
				audioProvider_ = undefined;
				audioProcessor_.disconnect();
				if (recorder_.TRACE) recorder_.TRACE("INFO: stopped recording");
			}
		};
		audioProcessor_onaudioprocess_downSampling_ = function (event) {
			// <!-- for Safari and AudioWorklet
			if (state_ === 0) {
				return;
			}
			// -->
			var audioData = event.data;
			var audioDataIndex = 0;
			while (temporaryAudioDataSamples_ < temporaryAudioData_.length) {
				temporaryAudioData_[temporaryAudioDataSamples_++] = audioData[audioDataIndex++];
			}
			while (temporaryAudioDataSamples_ == temporaryAudioData_.length) {
				var pcmDataBytes = (audioData.length / audioDecimationFactor_ | 0) * 2;
				var pcmData = new Uint8Array(new ArrayBuffer(16 + pcmDataBytes), 16, pcmDataBytes);
				var pcmDataIndex = 0;
				for (var temporaryAudioDataIndex = audioDecimationFactor_ - 1; temporaryAudioDataIndex + 20 < temporaryAudioData_.length; temporaryAudioDataIndex += audioDecimationFactor_) {
					var pcm_float = 0.0;
					for (var i = 0; i <= 20; i++) {
						pcm_float += temporaryAudioData_[temporaryAudioDataIndex + i] * coefData_[i];
					}
					var pcm = pcm_float * 32768 | 0; // 小数 (0.0～1.0) を 整数 (-32768～32767) に変換...
					if (pcm > 32767) {
						pcm = 32767;
					} else
					if (pcm < -32768) {
						pcm = -32768;
					}
					pcmData[pcmDataIndex++] = (pcm     ) & 0xFF;
					pcmData[pcmDataIndex++] = (pcm >> 8) & 0xFF;
				}
				if (!encoder_) {
					waveData_.push(pcmData);
				} else {
					pcmData = encoder_.encode(pcmData);
					if (pcmData) {
						waveData_.push(pcmData);
					}
				}
				temporaryAudioDataSamples_ = 0;
				var temporaryAudioDataIndex = temporaryAudioData_.length - 20;
				while (temporaryAudioDataIndex < temporaryAudioData_.length) {
					temporaryAudioData_[temporaryAudioDataSamples_++] = temporaryAudioData_[temporaryAudioDataIndex++];
				}
				while (audioDataIndex < audioData.length) {
					temporaryAudioData_[temporaryAudioDataSamples_++] = audioData[audioDataIndex++];
				}
			}
			if (state_ === 3) {
				state_ = 4;
				audioStream_.stopTracks();
				audioStream_ = undefined;
				audioProvider_.disconnect();
				audioProvider_ = undefined;
				audioProcessor_.disconnect();
				if (recorder_.TRACE) recorder_.TRACE("INFO: stopped recording");
			}
		};
		audioProcessor_onaudioprocess_downSampling_recorded_ = function (event) {
			// <!-- for Safari and AudioWorklet
			if (state_ === 0) {
				return;
			}
			// -->
			var audioData = event.data;
			var audioDataIndex = 0;
			while (temporaryAudioDataSamples_ < temporaryAudioData_.length) {
				temporaryAudioData_[temporaryAudioDataSamples_++] = audioData[audioDataIndex++];
			}
			while (temporaryAudioDataSamples_ == temporaryAudioData_.length) {
				var pcmDataOffset = (encoder_) ? 1 + 16 : 1;
				var pcmDataIndex = pcmDataOffset;
				for (var temporaryAudioDataIndex = audioDecimationFactor_ - 1; temporaryAudioDataIndex + 20 < temporaryAudioData_.length; temporaryAudioDataIndex += audioDecimationFactor_) {
					var pcm_float = 0.0;
					for (var i = 0; i <= 20; i++) {
						pcm_float += temporaryAudioData_[temporaryAudioDataIndex + i] * coefData_[i];
					}
					var pcm = pcm_float * 32768 | 0; // 小数 (0.0～1.0) を 整数 (-32768～32767) に変換...
					if (pcm > 32767) {
						pcm = 32767;
					} else
					if (pcm < -32768) {
						pcm = -32768;
					}
					pcmData_[pcmDataIndex++] = (pcm     ) & 0xFF;
					pcmData_[pcmDataIndex++] = (pcm >> 8) & 0xFF;
				}
				var pcmData = pcmData_.subarray(pcmDataOffset, pcmDataIndex);
				if (!encoder_) {
					recorder_.recorded(pcmData);
				} else {
					pcmData = encoder_.encode(pcmData);
					if (pcmData) {
						recorder_.recorded(pcmData);
					}
				}
				temporaryAudioDataSamples_ = 0;
				var temporaryAudioDataIndex = temporaryAudioData_.length - 20;
				while (temporaryAudioDataIndex < temporaryAudioData_.length) {
					temporaryAudioData_[temporaryAudioDataSamples_++] = temporaryAudioData_[temporaryAudioDataIndex++];
				}
				while (audioDataIndex < audioData.length) {
					temporaryAudioData_[temporaryAudioDataSamples_++] = audioData[audioDataIndex++];
				}
			}
			if (state_ === 3) {
				state_ = 4;
				audioStream_.stopTracks();
				audioStream_ = undefined;
				audioProvider_.disconnect();
				audioProvider_ = undefined;
				audioProcessor_.disconnect();
				if (recorder_.TRACE) recorder_.TRACE("INFO: stopped recording");
			}
		};
		if (audioContext_.sampleRate === 48000) {
			audioSamplesPerSec_ = 16000;
			audioDecimationFactor_ = 3;
		} else
		if (audioContext_.sampleRate === 44100) {
			audioSamplesPerSec_ = 22050;
			audioDecimationFactor_ = 2;
		} else
		if (audioContext_.sampleRate === 22050) {
			audioSamplesPerSec_ = 22050;
			audioDecimationFactor_ = 1;
		} else
		if (audioContext_.sampleRate === 16000) {
			audioSamplesPerSec_ = 16000;
			audioDecimationFactor_ = 1;
		} else
		if (audioContext_.sampleRate === 8000) {
			audioSamplesPerSec_ = 8000;
			audioDecimationFactor_ = 1;
		} else {
			audioSamplesPerSec_ = 0;
			audioDecimationFactor_ = 0;
		}
		if (audioDecimationFactor_ > 1) {
			temporaryAudioData_ = new Float32Array(20 + ((audioProcessor_.bufferSize / audioDecimationFactor_ >> 1) << 1) * audioDecimationFactor_);
			temporaryAudioDataSamples_ = 0;
			coefData_ = new Float32Array(10 + 1 + 10);
			if (audioDecimationFactor_ == 3) {
				coefData_[ 0] = -1.9186907e-2;
				coefData_[ 1] =  1.2144312e-2;
				coefData_[ 2] =  3.8677038e-2;
				coefData_[ 3] =  3.1580867e-2;
				coefData_[ 4] = -1.2342449e-2;
				coefData_[ 5] = -6.0144741e-2;
				coefData_[ 6] = -6.1757100e-2;
				coefData_[ 7] =  1.2462522e-2;
				coefData_[ 8] =  1.4362448e-1;
				coefData_[ 9] =  2.6923548e-1;
				coefData_[10] =  3.2090380e-1;
				coefData_[11] =  2.6923548e-1;
				coefData_[12] =  1.4362448e-1;
				coefData_[13] =  1.2462522e-2;
				coefData_[14] = -6.1757100e-2;
				coefData_[15] = -6.0144741e-2;
				coefData_[16] = -1.2342449e-2;
				coefData_[17] =  3.1580867e-2;
				coefData_[18] =  3.8677038e-2;
				coefData_[19] =  1.2144312e-2;
				coefData_[20] = -1.9186907e-2;
			} else {
				coefData_[ 0] =  6.91278819431317970157e-6;
				coefData_[ 1] =  3.50501872599124908447e-2;
				coefData_[ 2] = -6.93948777552577666938e-6;
				coefData_[ 3] = -4.52254377305507659912e-2;
				coefData_[ 4] =  6.96016786605468951166e-6;
				coefData_[ 5] =  6.34850487112998962402e-2;
				coefData_[ 6] = -6.97495897838962264359e-6;
				coefData_[ 7] = -1.05997055768966674805e-1;
				coefData_[ 8] =  6.98394205755903385580e-6;
				coefData_[ 9] =  3.18274468183517456055e-1;
				coefData_[10] =  4.99993026256561279297e-1;
				coefData_[11] =  3.18274468183517456055e-1;
				coefData_[12] =  6.98394205755903385580e-6;
				coefData_[13] = -1.05997055768966674805e-1;
				coefData_[14] = -6.97495897838962264359e-6;
				coefData_[15] =  6.34850487112998962402e-2;
				coefData_[16] =  6.96016786605468951166e-6;
				coefData_[17] = -4.52254377305507659912e-2;
				coefData_[18] = -6.93948777552577666938e-6;
				coefData_[19] =  3.50501872599124908447e-2;
				coefData_[20] =  6.91278819431317970157e-6;
			}
		}
		pcmData_ = new Uint8Array(1 + 16 + audioProcessor_.bufferSize * 2);
		reason_ = {code: 0, message: ""};
		maxRecordingTimeTimerId_ = null;
	}

	// 録音の開始
	async function resume_() {
		if (state_ !== -1 && state_ !== 0) {
			if (recorder_.TRACE) recorder_.TRACE("ERROR: can't start recording (invalid state: " + state_ + ")");
			return false;
		}
		if (recorder_.resumeStarted) recorder_.resumeStarted();
		if (!window.AudioContext) {
			if (recorder_.TRACE) recorder_.TRACE("ERROR: can't start recording (Unsupported AudioContext class)");
			if (recorder_.pauseEnded) recorder_.pauseEnded({code: 2, message: "Unsupported AudioContext class"}, waveFile_);
			return true;
		}
		if (!navigator.mediaDevices) {
			if (recorder_.TRACE) recorder_.TRACE("ERROR: can't start recording (Unsupported MediaDevices class)");
			if (recorder_.pauseEnded) recorder_.pauseEnded({code: 2, message: "Unsupported MediaDevices class"}, waveFile_);
			return true;
		}
		if (recorder_.recordingSourceElement) recorder_.recordingSource = recorder_.recordingSourceElement.value - 0;
		if (recorder_.sampleRateElement) recorder_.sampleRate = recorder_.sampleRateElement.value - 0;
		if (recorder_.maxRecordingTimeElement) recorder_.maxRecordingTime = recorder_.maxRecordingTimeElement.value - 0;
		if (recorder_.downSamplingElement) recorder_.downSampling = recorder_.downSamplingElement.checked;
		if (recorder_.adpcmPackingElement) recorder_.adpcmPacking = recorder_.adpcmPackingElement.checked;
		if (recorder_.opusPackingElement) recorder_.opusPacking = recorder_.opusPackingElement.checked;
		if (state_ === 0) {
			audioStream_ = null;
			audioProvider_ = null;
			audioProcessor_ = null;
			audioContext_.close();
			audioContext_ = null;
			state_ = -1;
		}
		if (state_ === -1) {
			// 各種変数の初期化
			await initialize_();
			state_ = 0;
		}
		if (recorder_.downSampling) {
			if (audioContext_.sampleRate === 48000) {
				audioSamplesPerSec_ = 16000;
				audioDecimationFactor_ = 3;
			} else
			if (audioContext_.sampleRate === 44100) {
				audioSamplesPerSec_ = 22050;
				audioDecimationFactor_ = 2;
			} else
			if (audioContext_.sampleRate === 22050) {
				audioSamplesPerSec_ = 22050;
				audioDecimationFactor_ = 1;
			} else
			if (audioContext_.sampleRate === 16000) {
				audioSamplesPerSec_ = 16000;
				audioDecimationFactor_ = 1;
			} else
			if (audioContext_.sampleRate === 8000) {
				audioSamplesPerSec_ = 8000;
				audioDecimationFactor_ = 1;
			} else {
				audioSamplesPerSec_ = 0;
				audioDecimationFactor_ = 0;
			}
		} else {
			audioSamplesPerSec_ = audioContext_.sampleRate;
			audioDecimationFactor_ = 1;
		}
		if (audioSamplesPerSec_ === 0) {
			if (recorder_.TRACE) recorder_.TRACE("ERROR: can't start recording (Unsupported sample rate: " + audioContext_.sampleRate + "Hz)");
			reason_.code = 2;
			reason_.message = "Unsupported sample rate: " + audioContext_.sampleRate + "Hz";
			if (recorder_.pauseEnded) recorder_.pauseEnded(reason_, waveFile_);
			return true;
		}
		state_ = 1;
		if (audioDecimationFactor_ > 1) {
			for (var i = 0; i <= 20; i++) {
				temporaryAudioData_[i] = 0.0;
			}
			temporaryAudioDataSamples_ = 20;
		}
		// <!-- for ADPCM/Opus packing
		encoder_ = null;
		if (!encoder_ && recorder_.opusPacking && window.Encoder && window.Encoder.createOpusEncoder) {
			encoder_ = window.Encoder.createOpusEncoder(audioSamplesPerSec_, /* bitsPerSec */ 24000, /* vbr: VBR */ 1, /* application: AUDIO */ 1, /* complexity */ 10);
		}
		if (!encoder_ && recorder_.adpcmPacking && window.Encoder && window.Encoder.createADPCMEncoder) {
			encoder_ = window.Encoder.createADPCMEncoder(audioSamplesPerSec_);
		}
		converter_ = null;
		if (encoder_ && encoder_.convert) {
			converter_ = encoder_;
		}
		if (recorder_.TRACE) if (encoder_) recorder_.TRACE("INFO: prepared encoder: " + encoder_.version);
		// -->
		waveData_ = [];
		waveFile_ = null;
		reason_.code = 0;
		reason_.message = "";
		if (audioDecimationFactor_ > 1) {
			if (recorder_.recorded) {
				audioProcessor_.port.onmessage = audioProcessor_onaudioprocess_downSampling_recorded_;
			} else {
				audioProcessor_.port.onmessage = audioProcessor_onaudioprocess_downSampling_;
			}
		} else {
			if (recorder_.recorded) {
				audioProcessor_.port.onmessage = audioProcessor_onaudioprocess_recorded_;
			} else {
				audioProcessor_.port.onmessage = audioProcessor_onaudioprocess_;
			}
		}
		const stopTracks = function () {
			var tracks = this.getTracks();
			for (var i = 0; i < tracks.length; i++) {
				tracks[i].stop();
			}
			if (!recorder_.recorded) {
				if (!encoder_) {
					var waveDataBytes = 0;
					for (var i = 0; i < waveData_.length; i++) {
						waveDataBytes += waveData_[i].byteLength;
					}
					var waveData = new Uint8Array(44);
					waveData[ 0] = 0x52; // 'R'
					waveData[ 1] = 0x49; // 'I'
					waveData[ 2] = 0x46; // 'F'
					waveData[ 3] = 0x46; // 'F'
					waveData[ 4] = (waveDataBytes + 36) & 0xFF;
					waveData[ 5] = ((waveDataBytes + 36) >> 8) & 0xFF;
					waveData[ 6] = ((waveDataBytes + 36) >> 16) & 0xFF;
					waveData[ 7] = ((waveDataBytes + 36) >> 24) & 0xFF;
					waveData[ 8] = 0x57; // 'W'
					waveData[ 9] = 0x41; // 'A'
					waveData[10] = 0x56; // 'V'
					waveData[11] = 0x45; // 'E'
					waveData[12] = 0x66; // 'f'
					waveData[13] = 0x6D; // 'm'
					waveData[14] = 0x74; // 't'
					waveData[15] = 0x20; // ' '
					waveData[16] = 16;
					waveData[17] = 0;
					waveData[18] = 0;
					waveData[19] = 0;
					waveData[20] = 1; // formatTag
					waveData[21] = 0;
					waveData[22] = 1; // channels
					waveData[23] = 0;
					waveData[24] = audioSamplesPerSec_ & 0xFF; // samplesPerSec
					waveData[25] = (audioSamplesPerSec_ >> 8) & 0xFF;
					waveData[26] = (audioSamplesPerSec_ >> 16) & 0xFF;
					waveData[27] = (audioSamplesPerSec_ >> 24) & 0xFF;
					waveData[28] = (audioSamplesPerSec_ * 2 * 1) & 0xFF; // bytesPseSec
					waveData[29] = ((audioSamplesPerSec_ * 2 * 1) >> 8) & 0xFF;
					waveData[30] = ((audioSamplesPerSec_ * 2 * 1) >> 16) & 0xFF;
					waveData[31] = ((audioSamplesPerSec_ * 2 * 1) >> 24) & 0xFF;
					waveData[32] = 2 * 1; // bytesPerSample
					waveData[33] = 0;
					waveData[34] = 16; // bitsPerSample
					waveData[35] = 0;
					waveData[36] = 0x64; // 'd'
					waveData[37] = 0x61; // 'a'
					waveData[38] = 0x74; // 't'
					waveData[39] = 0x61; // 'a'
					waveData[40] = waveDataBytes & 0xFF;
					waveData[41] = (waveDataBytes >> 8) & 0xFF;
					waveData[42] = (waveDataBytes >> 16) & 0xFF;
					waveData[43] = (waveDataBytes >> 24) & 0xFF;
					waveData_.unshift(waveData);
					waveFile_ = new Blob(waveData_, {type: "audio/wav"});
					waveFile_.samples = waveDataBytes / (2 * 1);
					waveFile_.samplesPerSec = audioSamplesPerSec_;
					waveFile_.extension = ".wav";
				} else {
					if (converter_) {
						waveData_ = converter_.convert(waveData_);
					}
					waveFile_ = new Blob(waveData_, {type: encoder_.mime});
					waveFile_.samples = encoder_.samples;
					waveFile_.samplesPerSec = encoder_.samplesPerSec;
					waveFile_.extension = encoder_.extension;
				}
				waveData_ = null;
			}
			if (encoder_) {
				encoder_.destroy();
				encoder_ = null;
			}
			state_ = 0;
			if (recorder_.pauseEnded) recorder_.pauseEnded(reason_, waveFile_);
		};
		try {
			if (recorder_.recordingSource < 1 || recorder_.recordingSource > 3) {
				recorder_.recordingSource = 1;
			}
			let audioStream;
			if (recorder_.recordingSource === 1) {
				audioStream = await navigator.mediaDevices.getUserMedia({audio: {echoCancellation: false}, video: false});
				audioStream.stopTracks = stopTracks;
			} else
			if (recorder_.recordingSource === 2) {
				audioStream = await navigator.mediaDevices.getDisplayMedia({audio: {echoCancellation: false}, video: true});
				audioStream.stopTracks = stopTracks;
			} else {
				audioStream = new MediaStream();
				audioStream.userMedia_ = await navigator.mediaDevices.getUserMedia({audio: {echoCancellation: false}, video: false});
				audioStream.displayMedia_ = await navigator.mediaDevices.getDisplayMedia({audio: {echoCancellation: false}, video: true});
				const destination = audioContext_.createMediaStreamDestination();
				audioContext_.createMediaStreamSource(audioStream.userMedia_).connect(destination);
				audioContext_.createMediaStreamSource(audioStream.displayMedia_).connect(destination);
				audioStream.addTrack(destination.stream.getTracks()[0]);
				audioStream.stopTracks_ = stopTracks;
				audioStream.stopTracks = function () {
					this.userMedia_.getTracks().forEach(track => track.stop());
					this.displayMedia_.getTracks().forEach(track => track.stop());
					this.stopTracks_();
				}
			}
			if (state_ === 3) {
				state_ = 4;
				audioStream.stopTracks();
				if (audioDecimationFactor_ > 1) {
					if (recorder_.TRACE) recorder_.TRACE("INFO: cancelled recording: " + audioContext_.sampleRate + "Hz -> " + audioSamplesPerSec_ + "Hz (" + audioProcessor_.bufferSize + " samples/buffer)");
				} else {
					if (recorder_.TRACE) recorder_.TRACE("INFO: cancelled recording: " + audioSamplesPerSec_ + "Hz (" + audioProcessor_.bufferSize + " samples/buffer)");
				}
				return false;
			}
			state_ = 2;
			audioStream_ = audioStream;
			audioProvider_ = audioContext_.createMediaStreamSource(audioStream_);
			audioProvider_.connect(audioProcessor_);
			audioProcessor_.connect(audioContext_.destination);
			if (audioDecimationFactor_ > 1) {
				if (recorder_.TRACE) recorder_.TRACE("INFO: started recording: " + audioContext_.sampleRate + "Hz -> " + audioSamplesPerSec_ + "Hz (" + audioProcessor_.bufferSize + " samples/buffer)");
			} else {
				if (recorder_.TRACE) recorder_.TRACE("INFO: started recording: " + audioSamplesPerSec_ + "Hz (" + audioProcessor_.bufferSize + " samples/buffer)");
			}
			startMaxRecordingTimeTimer_();
			var bytesPerSec = (encoder_) ? encoder_.bytesPerSec : audioSamplesPerSec_ * 2;
			var framesPerSec = (encoder_) ? encoder_.framesPerSec : 0;
			if (recorder_.resumeEnded) recorder_.resumeEnded(((encoder_) ? "" : "LSB") + (audioSamplesPerSec_ / 1000 | 0) + "K", bytesPerSec, framesPerSec);
		} catch (error) {
			state_ = 0;
			if (recorder_.TRACE) recorder_.TRACE("ERROR: can't start recording (" + error.message + ")");
			reason_.code = 2;
			reason_.message = error.message;
			if (recorder_.pauseEnded) recorder_.pauseEnded(reason_, waveFile_);
		}
		return true;
	}

	// 録音の停止
	function pause_() {
		if (state_ !== 2) {
			if (recorder_.TRACE) recorder_.TRACE("ERROR: can't stop recording (invalid state: " + state_ + ")");
			return false;
		}
		state_ = 3;
		if (recorder_.pauseStarted) recorder_.pauseStarted();
		stopMaxRecordingTimeTimer_();
		return true;
	}

	// 録音中かどうかの取得
	function isActive_() {
		return (state_ === 2);
	}

	// コンバーダの取得
	function getConverter_() {
		return converter_;
	}

	// 録音の停止を自動的に行うためのタイマの開始
	function startMaxRecordingTimeTimer_() {
		if (recorder_.maxRecordingTime <= 0) {
			return;
		}
		stopMaxRecordingTimeTimer_();
		maxRecordingTimeTimerId_ = setTimeout(fireMaxRecordingTimeTimer_, recorder_.maxRecordingTime);
		if (recorder_.TRACE) recorder_.TRACE("INFO: started auto pause timeout timer: " + recorder_.maxRecordingTime);
	}

	// 録音の停止を自動的に行うためのタイマの停止
	function stopMaxRecordingTimeTimer_() {
		if (maxRecordingTimeTimerId_ !== null) {
			clearTimeout(maxRecordingTimeTimerId_);
			maxRecordingTimeTimerId_ = null;
			if (recorder_.TRACE) recorder_.TRACE("INFO: stopped auto pause timeout timer: " + recorder_.maxRecordingTime);
		}
	}

	// 録音の停止を自動的に行うためのタイマの発火
	function fireMaxRecordingTimeTimer_() {
		if (recorder_.TRACE) recorder_.TRACE("INFO: fired auto pause timeout timer: " + recorder_.maxRecordingTime);
		reason_.code = 1;
		reason_.message = "Exceeded max recording time";
		pause_();
	}

	// public プロパティの初期化
	if (window.Encoder && window.Encoder.ADPCMEncoder) {
		recorder_.version += " " + window.Encoder.ADPCMEncoder.version;
	}
	if (window.Encoder && window.Encoder.OpusEncoder) {
		recorder_.version += " " + window.Encoder.OpusEncoder.version;
	}

	// public オブジェクトの返却
	return recorder_;
}();
