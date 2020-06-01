<?php

namespace com\amivoice\hrp;

interface HrpListener {
	function resultCreated($sessionId);
	function resultUpdated($result);
	function resultFinalized($result);
	function TRACE($message);
}

?>
