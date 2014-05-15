<?php

function _dossiersearch_getresults($query='') {
  $nodes    = array();
  $queries  = array();

  $pQueries = explode(';',$query);
  foreach ($pQueries as $query) {
    $query  = explode('=',$query);
    $queries[$query[0]] = isset($query[1])?$query[1]:NULL;
  }
  // Clean Input
  $queries['title']   = _dossiersearch_cleanTitle       ($queries['title']) ;
  $queries['bounds']  = _dossiersearch_cleanBounds      ($queries['bounds']);
  $queries['terms']   = _dossiersearch_cleanTerms       ($queries['terms']) ;
  $queries['date']    = _dossiersearch_cleanDate        ($queries['date'])  ;
  $queries['group']   = (int)                            $queries['group']  ;

  // Create SQL Strings
  $queries['qTitle']  = _dossiersearch_createQueryTitle ($queries['title']) ;
  $queries['qDate']   = _dossiersearch_createQueryDate  ($queries['date'])  ;
  $queries['qBounds'] = _dossiersearch_createQueryBounds($queries['bounds']);


  // get nodes, that match the Term Criteria
  // TODO: vid werden im Moment auch noch beachtet; es sollte jedoch nur die aktuellste Version angeschaut werden, oder?
  // TODO: Point in Polygon wenn Projektgebiet ausgewählt und Karte ausgeschaltet
  $nodesTerms = _dossiersearch_FilterTerms($queries['terms']);

  // Get nodes that match Title Filter Criteria
  $sql =
   'SELECT n.nid FROM {node} n
    LEFT JOIN {content_type_dossier} cd
    ON (n.vid = cd.vid)
    WHERE
       n.type LIKE \'dossier\' '.
      'AND cd.field_papierkorb_value LIKE \'normal\' '.
      $queries['qTitle'].$queries['qDate'];
  //dsm(str_replace(array('{','}'),'',$sql));
  $results = db_query(db_rewrite_sql($sql),$queries['title']);
  while ($r = db_fetch_object($results)) {
    if (isset($nodesTerms[$r->nid]) || !$nodesTerms) {
      $nodes[$r->nid] = $r->nid;
    }
  }

  // Sucht nach Dateinamen mit Titelfeld
  if ($queries['title']) {
    $sql = "".
     "SELECT {content_field_dateien}.nid FROM {files} ".
       "LEFT JOIN {content_field_dateien} ".
         "LEFT JOIN {node} n ".
           "ON ({content_field_dateien}.nid = n.nid) ".
         "ON ({files}.fid = {content_field_dateien}.field_dateien_fid) ".
     "WHERE {content_field_dateien}.nid IS NOT NULL "._dossiersearch_createQueryFilename($queries['title']);
    //dsm(str_replace(array('{','}'),'',$sql));
    $results = db_query(db_rewrite_sql($sql), $queries['title']);
    while ($r = db_fetch_object($results)) {
      if (isset($nodesTerms[$r->nid]) || !$nodesTerms) {
        $nodes[$r->nid] = $r->nid;
      }
    }
  }


  // Sucht nach Autorengruppe
  if ($queries['group']) {
    $nodes_prae = $nodes;
    $nodes = array();
    $sql = "".
     "SELECT n.nid FROM {node} n ".
       "RIGHT JOIN {og_uid} ".
       "ON (n.uid = {og_uid}.uid) ".
     "WHERE {og_uid}.nid = '%d' AND n.type = 'dossier' AND n.uid != 1";
    //dsm(str_replace(array('{','}'),'',$sql));
    $results = db_query(db_rewrite_sql($sql),$queries['group']);
    while ($r = db_fetch_object($results)) {
      if (isset($nodes_prae[$r->nid])) {
        $nodes[$r->nid] = $r->nid;
      }
    }

  }

  // Get nodes that do NOT match bounds criteria and remove them from searchresults
  if ($queries['bounds'] && $queries['mapactive']=='true') {
    $results = db_query(db_rewrite_sql($queries['qBounds']));
    while ($r = db_fetch_object($results)) {
      if (isset($nodes[$r->nid])) unset($nodes[$r->nid]);
    }
  }
  return $nodes;
}


/**
 * Cleans SearchString
 */
function _dossiersearch_cleanTitle($title) {
  $title = trim(strtolower($title));
  $title = preg_replace('/[^a-zäöü0-9\-\.\s_]/', '', $title);
  $title = preg_replace('|\s+|', ' ', $title);
  $title = explode(' ',$title);
  foreach ($title as $key=>$t) {
    if ($t == '') unset($title[$key]);
  }
  if (!count($title)) return false;
  $title = array_values($title); // resets array-keys
  return $title;
}

/**
 * Cleans Date From->To
 * arg $date = string '123456789,987654321'
 * @return     array  Bsp: <i>array(123456789,987654321)</i>
 */
function _dossiersearch_cleanDate($date) {
  $days = explode(',',$date);
  if (count($days) != 2)              return false;
  if ($days[0] == 0 || $days[1] == 0) return false;
  return array(preg_replace('|[^0-9]|','',$days[0]),
               preg_replace('|[^0-9]|','',$days[1]));
}


/**
 * Cleans Boundries Data<br />
 * returns formated boundries as array
 * @param    string $bounds '47.1243,7.1234,46.5678,7.5678'
 * @return   array  array  (47.1243,7.1234,46.5678,7.5678),
 * gibt false zurück, wenn ungültige Eingabe übergeben wurde
 */
function _dossiersearch_cleanBounds($bounds) {
  // Wenn nicht nach Lokalität gefilter werden kann false zurückgeben
  if (!variable_get('dossiersearch_filterlocation', false)) return false;
  $bounds = trim(preg_replace('|[^0-9,\.]|','',$bounds));
  $bounds = explode(',',$bounds);
  if (count($bounds)!=4) return false;
  return $bounds;
}

/**
 * Cleans Term-ID Data
 * return formated Term-Id, exp:
 * @param string $tids  '23,0,12'   -> returns array(23,0,12);
 * @param string $tids  'abac'       -> returns false/0
 * @param string $tids  '12'         -> returns array(12);
 * @return array array(tid1, tid2, etc.), 0/false if no valid terms given
 */
function _dossiersearch_cleanTerms($tids) {
  $terms = explode(',', $tids);
  foreach ($terms as $key=>$term) {
    $terms[$key] = (int) $term;
  }
  return $terms;
}

/**
 * Creates SQL Filter for Searchstring
 * @param array $title array('searchstring1', 'searchstrin2', ...)
 * @return string <i>AND n.title LIKE '%%%s%%' AND n.title LIKE '%%%s%%'</i> etc.
 */
function _dossiersearch_createQueryTitle($title) {
  if (!$title) return '';
  $q = array();
  foreach ($title as $t) {
    $q[] = "n.title LIKE '%%%s%%'";
  }
  return ' AND ('.implode(' AND ', $q).') ';
}

/**
 * Creates SQL Filter for Filename
 * @param array $title array('searchstring1', 'searchstrin2', ...)
 * @return string <i>AND n.title LIKE '%%%s%%' AND n.title LIKE '%%%s%%'</i> etc.
 */
function _dossiersearch_createQueryFilename($title) {
  if (!$title) return '';
  $q = array();
  foreach ($title as $t) {
    $q[] = "{files}.filename LIKE '%%%s%%'";
  }
  return ' AND ('.implode(' AND ', $q).') ';
}


/**
 * Creates SQL Filter for Bounds
 * @param array $bounds array(x1,y1,x2,y2)
 * @return string 'SELECT n.nid FROM node n ...', Query um alle nodes
 * auszuwählen, welche ausserhalb von Bounds liegen
 */
function _dossiersearch_createQueryBounds($bounds) {
  if (!$bounds) return '';
  return 'SELECT n.nid FROM {node} n
            LEFT JOIN {location_instance} li ON (n.vid = li.vid)
            LEFT JOIN {location} l ON (l.lid=li.lid)
          WHERE
            ((l.latitude IS NOT NULL)
           AND
            ((l.latitude > '.$bounds[2].' OR l.latitude < '.$bounds[0].')
           OR
            (l.longitude > '.$bounds[3].' OR l.longitude < '.$bounds[1].'))
            )
           OR
            (l.latitude IS NULL)';
}

/**
 * Creates SQL Filter for Date
 * @param array $date array(123456789, 987654321) => (timestamp_from, timestamp_to)
 * @return string <i>' AND ((cd.field_dokumentendatum_value >= '1234567' AND
 * cd.field_dokumentendatum_value <= '7654321') OR (n.changed >= 1234567
 * AND n.changed <=7654321) OR (n.created >= 1234567 AND n.created <=7654321))'</i>
 */
function _dossiersearch_createQueryDate($date) {
  if (!$date) return '';

  $datesql = array();
  $from = date("Y-m-d\T00:00:00",$date[0]);
  $to   = date("Y-m-d\T23:59:59",$date[1]);

  $datesql[] = "(cd.field_dokumentendatum_value >= '".$from."' AND cd.field_dokumentendatum_value <= '".$to."')";
  $datesql[] = "(n.changed >= ".$date[0]." AND n.changed <= ".($date[1]+60*60*24-1).")";
  $datesql[] = "(n.created >= ".$date[0]." AND n.created <= ".($date[1]+60*60*24-1).")";

  return ' AND ('.implode(' OR ',$datesql).')';
}


/**
 * Creates SQL Filter for Terms
 * $tids = array(1,2,3,4)
 * return $out array(1=>1,nid=>nid,...)
 */
function _dossiersearch_FilterTerms($tids) {
  // Nullwerte löschen
  foreach ($tids as $key=>$tid) {
    if (!$tid) unset($tids[$key]);
  }

  // keine Filterung nach Terms
  if (!count($tids)) return false;

// Reset array key
  $tids = array_values($tids);

  $SELECT = array();
  // Temporäre Tabelle von jedem Term
  foreach ($tids as $key=>$tid) {
    $SELECT[$key] = '(SELECT nid FROM {term_node} WHERE tid = '.$tid.') AS term_node_'.$key;
  }
  $sql = 'SELECT term_node_0.nid FROM '.$SELECT[0];
  foreach ($tids as $key=>$tid) {
    if ($key == 0) continue;
      $sql .= ' INNER JOIN '.$SELECT[$key].' ON term_node_0.nid = term_node_'.$key.'.nid';
  }

  $out = array();
  $results = db_query($sql);
  while ($r = db_fetch_object($results)) {
    $out[$r->nid] = $r->nid;
  }
  return count($out)?$out:false;
}


/*

Basel West
<coordinates>
  7.575889,47.542063,0
  7.575889,47.546147,0
  7.581532,47.546147,0
  7.581532,47.542179,0
  7.575889,47.542063,0
</coordinates>

Basel Zentrum
<coordinates>
  7.582219,47.54496,0
  7.577262,47.54496,0
  7.577262,47.548812,0
  7.582133,47.548812,0
  7.582219,47.54496,0
</coordinates>

Tunnel
<coordinates>
  7.578335,47.544786,0
  7.579536,47.546727,0
  7.581854,47.547741,0
  7.583699,47.54832,0
  7.600522,47.544902,0
  7.603655,47.546292,0
  7.605457,47.544757,0
  7.598248,47.542468,0
  7.578335,47.544786,0
</coordinates>

Basel Süd
<coordinates>
  7.602067,47.543337,0
  7.602067,47.546872,0
  7.61065,47.546872,0
  7.61065,47.543337,0
  7.602067,47.543337,0
</coordinates>
*/









