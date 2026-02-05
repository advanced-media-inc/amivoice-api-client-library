window.Encoder = window.Encoder || {};

window.Encoder.OpusEncoder = window.Encoder.OpusEncoder || {version: "OpusEncoder/1.0.11"};

var Module = {};

window.Encoder.createOpusEncoder = function (samplesPerSec, bitsPerSec, vbr, application, complexity) {
	// <!--
	if (bitsPerSec < samplesPerSec / 4) {
		bitsPerSec = samplesPerSec / 4;
	}
	bitsPerSec = (bitsPerSec / 400 | 0) * 400;
	if (bitsPerSec >= 102400 && vbr == /* hard-CBR */ 0) {
		vbr = /* VBR */ 1;
	}
	// -->
	var value = Module._malloc(4);
	var encoder_ = Module._opus_encoder_create(samplesPerSec, /* channels */ 1, (application == /* AUDIO */ 1) ? /* OPUS_APPLICATION_AUDIO */ 2049 : /* OPUS_APPLICATION_VOIP */ 2048, value);
	if (Module.HEAP32[value / 4] != /* OPUS_OK */ 0 || encoder_ == 0) {
		if (encoder_) {
			Module._opus_encoder_destroy(encoder_);
			encoder_ = 0;
		}
		Module._free(value);
		return null;
	}

	var samplesPerSec_ = samplesPerSec;
	var bitsPerSec_ = bitsPerSec;
	var vbr_ = vbr;
	var application_ = application;
	var complexity_ = complexity;
	var samplesPerFrame_ = samplesPerSec_ / 50;
	var bytesPerFrame_ = (vbr_ == /* hard-CBR */ 0) ? bitsPerSec_ / 400 : 0;
	var maxBytesPerFrame_ = (vbr_ == /* hard-CBR */ 0) ? bitsPerSec_ / 400 : samplesPerFrame_ * 1;
	Module._opus_encoder_ctl_(encoder_, /* OPUS_SET_BITRATE_REQUEST */ 4002, bitsPerSec_);
	if (vbr_ == /* hard-CBR */ 0) {
		Module._opus_encoder_ctl_(encoder_, /* OPUS_SET_VBR_REQUEST */ 4006, /* VBR */ 0);
	} else
	if (vbr_ == /* VBR */ 1) {
		Module._opus_encoder_ctl_(encoder_, /* OPUS_SET_VBR_REQUEST */ 4006, /* VBR */ 1);
		Module._opus_encoder_ctl_(encoder_, /* OPUS_SET_VBR_CONSTRAINT_REQUEST */ 4020, /* constrained-VBR */ 0);
	} else
	if (vbr_ == /* constrained-VBR */ 2) {
		Module._opus_encoder_ctl_(encoder_, /* OPUS_SET_VBR_REQUEST */ 4006, /* VBR */ 1);
		Module._opus_encoder_ctl_(encoder_, /* OPUS_SET_VBR_CONSTRAINT_REQUEST */ 4020, /* constrained-VBR */ 1);
	}
	Module._opus_encoder_ctl_(encoder_, /* OPUS_SET_COMPLEXITY_REQUEST */ 4010, complexity_);
	Module.HEAP32[value / 4] = 0;
	Module._opus_encoder_ctl_(encoder_, /* OPUS_GET_LOOKAHEAD_REQUEST */ 4027, value);
	var lookahead_ = Module.HEAP32[value / 4];
	Module._free(value);

	var header_ = 0;
	var pcmDataPointer_ = Module._malloc(samplesPerFrame_ * 2);
	var pcmData_ = Module.HEAP16.subarray(pcmDataPointer_ / 2, pcmDataPointer_ / 2 + samplesPerFrame_);
	var pcmDataSampleCount_ = 0;
	var opusDataPointer_ = Module._malloc(maxBytesPerFrame_);
	var opusData_ = Module.HEAP8.subarray(opusDataPointer_, opusDataPointer_ + maxBytesPerFrame_);

	function encode_(data) {
		var readingDataByteCount = data.byteLength;
		var readDataByteCount = 0;
		var writtenDataByteCount = 0;
		var dataView = new DataView(data.buffer, data.byteOffset, data.byteLength);
		// <!-- copy dataView to pcmData_
		var filled = false;
		while (readingDataByteCount >= 2) {
			var pcm = dataView.getInt16(readDataByteCount, true);
			readingDataByteCount -= 2;
			readDataByteCount += 2;
			pcmData_[pcmDataSampleCount_++] = pcm;
			if (pcmDataSampleCount_ >= samplesPerFrame_) {
				filled = true;
				break;
			}
		}
		// -->
		while (filled) {
			var opusDataByteCount = Module._opus_encode(encoder_, pcmDataPointer_, samplesPerFrame_, opusDataPointer_, maxBytesPerFrame_);
			// <!-- copy dataView to pcmData_
			filled = false;
			pcmDataSampleCount_ = 0;
			while (readingDataByteCount >= 2) {
				var pcm = dataView.getInt16(readDataByteCount, true);
				readingDataByteCount -= 2;
				readDataByteCount += 2;
				pcmData_[pcmDataSampleCount_++] = pcm;
				if (pcmDataSampleCount_ >= samplesPerFrame_) {
					filled = true;
					break;
				}
			}
			// -->
			if (opusDataByteCount > 0) {
				if (vbr_ != /* hard-CBR */ 0) {
					data[writtenDataByteCount++] = opusDataByteCount & 0xFF;
					data[writtenDataByteCount++] = (opusDataByteCount >> 8) & 0xFF;
				}
				// <!-- expand data if necessary
				if (writtenDataByteCount + opusDataByteCount > data.byteLength) {
					if (writtenDataByteCount + opusDataByteCount <= data.buffer.byteLength - data.byteOffset) {
						data = new Uint8Array(data.buffer, data.byteOffset, writtenDataByteCount + opusDataByteCount);
					} else {
						var newData = new Uint8Array(new ArrayBuffer(data.byteOffset + writtenDataByteCount + opusDataByteCount), data.byteOffset, writtenDataByteCount + opusDataByteCount);
						newData.set(data.subarray(0, writtenDataByteCount), 0);
						data = newData;
					}
				}
				// -->
				// <!-- copy opusData_ to data
				data.set(opusData_.subarray(0, opusDataByteCount), writtenDataByteCount);
				writtenDataByteCount += opusDataByteCount;
				// -->
				if (this.samples === 0) {
					this.samples += samplesPerFrame_ - lookahead_;
				} else {
					this.samples += samplesPerFrame_;
				}
			}
		}
		if (writtenDataByteCount == 0) {
			return null;
		}
		if (!header_) {
			data = new Uint8Array(data.buffer, data.byteOffset - 16, 16 + writtenDataByteCount);
			data[ 0] = 0x23; // '#'
			data[ 1] = 0x21; // '!'
			data[ 2] = 0x4F; // 'O'
			data[ 3] = 0x50; // 'P'
			data[ 4] = 0x53; // 'S'
			data[ 5] = 0x0A; // '\n'
			data[ 6] = samplesPerSec_ & 0xFF;
			data[ 7] = (samplesPerSec_ >> 8) & 0xFF;
			data[ 8] = 1;
			data[ 9] = 0;
			data[10] = complexity_;
			data[11] = lookahead_;
			data[12] = bytesPerFrame_;
			data[13] = samplesPerFrame_ & 0xFF;
			data[14] = (samplesPerFrame_ >> 8) & 0xFF;
			data[15] = 0;
			header_ = 1;
		} else {
			data = new Uint8Array(data.buffer, data.byteOffset, writtenDataByteCount);
		}
		return data;
	}

	function destroy_() {
		Module._free(pcmDataPointer_);
		Module._free(opusDataPointer_);
		Module._opus_encoder_destroy(encoder_);
		encoder_ = 0;
	}

	return {
		...window.Encoder.OpusEncoder,
		encode: encode_,
		destroy: destroy_,
		samples: 0,
		samplesPerSec: samplesPerSec_,
		bytesPerSec: bitsPerSec_ / 8,
		framesPerSec: samplesPerSec_ / samplesPerFrame_,
		mime: "audio/x-opus",
		extension: ".opus"
	};
};

if (window.Recorder) {
	window.Recorder.opusPacking = true;
}

var a;a||(a=typeof Module !== 'undefined' ? Module : {});var g={},h;for(h in a)a.hasOwnProperty(h)&&(g[h]=a[h]);function k(b,c){throw c;}var l=!1,m=!1,n=!1,p=!1;l="object"===typeof window;m="function"===typeof importScripts;n="object"===typeof process&&"object"===typeof process.versions&&"string"===typeof process.versions.node;p=!l&&!n&&!m;var r="",t,u,v,w,x;
if(n)r=m?require("path").dirname(r)+"/":__dirname+"/",t=function(b,c){w||(w=require("fs"));x||(x=require("path"));b=x.normalize(b);return w.readFileSync(b,c?null:"utf8")},v=function(b){b=t(b,!0);b.buffer||(b=new Uint8Array(b));b.buffer||y("Assertion failed: undefined");return b},1<process.argv.length&&process.argv[1].replace(/\\/g,"/"),process.argv.slice(2),"undefined"!==typeof module&&(module.exports=a),process.on("uncaughtException",function(b){if(!(b instanceof z))throw b;}),process.on("unhandledRejection",
y),k=function(b){process.exit(b)},a.inspect=function(){return"[Emscripten Module object]"};else if(p)"undefined"!=typeof read&&(t=function(b){return read(b)}),v=function(b){if("function"===typeof readbuffer)return new Uint8Array(readbuffer(b));b=read(b,"binary");"object"===typeof b||y("Assertion failed: undefined");return b},"function"===typeof quit&&(k=function(b){quit(b)}),"undefined"!==typeof print&&("undefined"===typeof console&&(console={}),console.log=print,console.warn=console.error="undefined"!==
typeof printErr?printErr:print);else if(l||m)m?r=self.location.href:"undefined"!==typeof document&&document.currentScript&&(r=document.currentScript.src),r=0!==r.indexOf("blob:")?r.substr(0,r.lastIndexOf("/")+1):"",t=function(b){var c=new XMLHttpRequest;c.open("GET",b,!1);c.send(null);return c.responseText},m&&(v=function(b){var c=new XMLHttpRequest;c.open("GET",b,!1);c.responseType="arraybuffer";c.send(null);return new Uint8Array(c.response)}),u=function(b,c,d){var f=new XMLHttpRequest;f.open("GET",
b,!0);f.responseType="arraybuffer";f.onload=function(){200==f.status||0==f.status&&f.response?c(f.response):d()};f.onerror=d;f.send(null)};a.print||console.log.bind(console);var A=a.printErr||console.warn.bind(console);for(h in g)g.hasOwnProperty(h)&&(a[h]=g[h]);g=null;a.quit&&(k=a.quit);var B;a.wasmBinary&&(B=a.wasmBinary);var noExitRuntime=a.noExitRuntime||!0;"object"!==typeof WebAssembly&&y("no native wasm support detected");var C,D=!1,E,F=[],G=[],H=[],I=[];
function J(){var b=a.preRun.shift();F.unshift(b)}var K=0,L=null,M=null;a.preloadedImages={};a.preloadedAudios={};function y(b){if(a.onAbort)a.onAbort(b);A(b);D=!0;throw new WebAssembly.RuntimeError("abort("+b+"). Build with -s ASSERTIONS=1 for more info.");}function N(){return O.startsWith("data:application/octet-stream;base64,")}var O="encoder_opus.wasm";if(!N()){var P=O;O=a.locateFile?a.locateFile(P,r):r+P}
function Q(){var b=O;try{if(b==O&&B)return new Uint8Array(B);if(v)return v(b);throw"both async and sync fetching of the wasm failed";}catch(c){y(c)}}
function R(){if(!B&&(l||m)){if("function"===typeof fetch&&!O.startsWith("file://"))return fetch(O,{credentials:"same-origin"}).then(function(b){if(!b.ok)throw"failed to load wasm binary file at '"+O+"'";return b.arrayBuffer()}).catch(function(){return Q()});if(u)return new Promise(function(b,c){u(O,function(d){b(new Uint8Array(d))},c)})}return Promise.resolve().then(function(){return Q()})}
function S(b){for(;0<b.length;){var c=b.shift();if("function"==typeof c)c(a);else{var d=c.h;"number"===typeof d?void 0===c.g?E.get(d)():E.get(d)(c.g):d(void 0===c.g?null:c.g)}}}var T={};
(function(){function b(e){a.asm=e.exports;C=a.asm.memory;e=C.buffer;a.HEAP8=new Int8Array(e);a.HEAP16=new Int16Array(e);a.HEAP32=new Int32Array(e);a.HEAPU8=new Uint8Array(e);a.HEAPU16=new Uint16Array(e);a.HEAPU32=new Uint32Array(e);a.HEAPF32=new Float32Array(e);a.HEAPF64=new Float64Array(e);E=a.asm.__indirect_function_table;K--;a.monitorRunDependencies&&a.monitorRunDependencies(K);0==K&&(null!==L&&(clearInterval(L),L=null),M&&(e=M,M=null,e()))}function c(e){b(e.instance)}function d(e){return R().then(function(q){return WebAssembly.instantiate(q,
f)}).then(e,function(q){A("failed to asynchronously prepare wasm: "+q);y(q)})}var f={env:T,wasi_snapshot_preview1:T};K++;a.monitorRunDependencies&&a.monitorRunDependencies(K);if(a.instantiateWasm)try{return a.instantiateWasm(f,b)}catch(e){return A("Module.instantiateWasm callback failed with error: "+e),!1}(function(){return B||"function"!==typeof WebAssembly.instantiateStreaming||N()||O.startsWith("file://")||"function"!==typeof fetch?d(c):fetch(O,{credentials:"same-origin"}).then(function(e){return WebAssembly.instantiateStreaming(e,
f).then(c,function(q){A("wasm streaming compile failed: "+q);A("falling back to ArrayBuffer instantiation");return d(c)})})})();return{}})();a._malloc=function(){return(a._malloc=a.asm.malloc).apply(null,arguments)};a._free=function(){return(a._free=a.asm.free).apply(null,arguments)};a._opus_encoder_create=function(){return(a._opus_encoder_create=a.asm.opus_encoder_create).apply(null,arguments)};a._opus_encode=function(){return(a._opus_encode=a.asm.opus_encode).apply(null,arguments)};
a._opus_encoder_ctl_=function(){return(a._opus_encoder_ctl_=a.asm.opus_encoder_ctl_).apply(null,arguments)};a._opus_encoder_destroy=function(){return(a._opus_encoder_destroy=a.asm.opus_encoder_destroy).apply(null,arguments)};a.__initialize=function(){return(a.__initialize=a.asm._initialize).apply(null,arguments)};var U;function z(){this.name="ExitStatus";this.message="Program terminated with exit(0)";this.status=0}M=function V(){U||W();U||(M=V)};
function W(){function b(){if(!U&&(U=!0,a.calledRun=!0,!D)){S(G);S(H);if(a.onRuntimeInitialized)a.onRuntimeInitialized();if(X){var c=a.__initialize;try{if(c(),!noExitRuntime){if(!noExitRuntime){if(a.onExit)a.onExit(0);D=!0}k(0,new z)}}catch(d){d instanceof z||"unwind"==d||((c=d)&&"object"===typeof d&&d.stack&&(c=[d,d.stack]),A("exception thrown: "+c),k(1,d))}finally{}}if(a.postRun)for("function"==typeof a.postRun&&(a.postRun=[a.postRun]);a.postRun.length;)c=a.postRun.shift(),I.unshift(c);S(I)}}if(!(0<
K)){if(a.preRun)for("function"==typeof a.preRun&&(a.preRun=[a.preRun]);a.preRun.length;)J();S(F);0<K||(a.setStatus?(a.setStatus("Running..."),setTimeout(function(){setTimeout(function(){a.setStatus("")},1);b()},1)):b())}}a.run=W;if(a.preInit)for("function"==typeof a.preInit&&(a.preInit=[a.preInit]);0<a.preInit.length;)a.preInit.pop()();var X=!0;a.noInitialRun&&(X=!1);W();
