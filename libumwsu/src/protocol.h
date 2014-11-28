#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_

/*
  Command: PING
  Headers: none
  Response: PONG
  Response headers: none
  Connexion: close

  Command: SCAN
  Headers:
    Path: <path_to_scan>
  Response: SCANNED
  Response headers:
    Path: <scanned_path> (is a file)
    Status: UNDECIDED | UNKNOWN | EINVAL | IERROR | SUSPICIOUS | MALWARE
    X-Status: <specific to scan module> (for instance virus name)
    Action: CLEAN | QUARANTINE | REMOVED | NONE
  Repeat response: yes
  Connexion: not closed
  Response: END
  Response headers: none
  Connexion: close

  Command: STATS
  Headers: none
  Response: STATS
  Response headers:
    Stat: line
    Repeat header: yes
  Connexion: close
  
later:
  Command: WATCH
  Headers:
    Path: <path_to_watch>
  Response: SCANNED
  idem SCAN, but no END response


Clamd examples:
$ echo "nVERSION" | netcat -U /var/run/clamav/clamd.ctl
ClamAV 0.98.1/19684/Wed Nov 26 15:29:58 2014
$ echo "nVERSIONCOMMANDS" | netcat -U /var/run/clamav/clamd.ctl
ClamAV 0.98.1/19684/Wed Nov 26 15:29:58 2014| COMMANDS: SCAN QUIT RELOAD PING CONTSCAN VERSIONCOMMANDS VERSION STREAM END SHUTDOWN MULTISCAN FILDES STATS IDSESSION INSTREAM DETSTATSCLEAR DETSTATS ALLMATCHSCAN

*/




#endif
