<?php

namespace com\amivoice\wrp;

interface WrpListener {
	function utteranceStarted($startTime);
	function utteranceEnded($endTime);
	function resultCreated();
	function resultUpdated($result);
	function resultFinalized($result);
	function TRACE($message);
}

?>
