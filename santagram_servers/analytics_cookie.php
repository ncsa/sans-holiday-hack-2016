<?php

# From crypto.php:

define('KEY', "\x61\x17\xa4\x95\xbf\x3d\xd7\xcd\x2e\x0d\x8b\xcb\x9f\x79\xe1\xdc");

function encrypt($data) {
  return mcrypt_encrypt(MCRYPT_ARCFOUR, KEY, $data, 'stream');
}

function decrypt($data) {
  return mcrypt_decrypt(MCRYPT_ARCFOUR, KEY, $data, 'stream');
}

# </crypto.php>

$auth = encrypt(json_encode([
  'username' => "administrator",
  'date' => date(DateTime::ISO8601),
]));
print  bin2hex($auth) . "\n";

?>
