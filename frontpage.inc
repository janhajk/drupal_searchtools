<?php


function frontpage_start() {
  drupal_add_css(drupal_get_path('module','frontpage').'/searchtools.css');
  drupal_add_js (drupal_get_path('module','frontpage').'/searchtools.js' );
  drupal_add_js (array('frontpage'=>array('searchpath' => variable_get('frontpage_searchpath','dossiersuche'))), 'setting');
  $frontpage = variable_get('frontpage_table','');
  if (time()-$frontpage[0] <= 24*60*60) {
    //return $frontpage[1];
  }
  $header = array(
      'Typ',
      'Titel',
      'Letzte Änderung',
      'Letzter Bearbeiter',
      'Zugriff',
      'Versionsinformation',

  );
  $nodes = frontpage_getData(30);
  $output = theme ('table', $header, $nodes, array('id'=>'frontpage_table'));
  variable_set('frontpage_table',array(time(),$output));
  if (!variable_get('frontpage_titel_hide', true)) {
    $output = '<h1>'.variable_get('frontpage_titel','Neustes').'</h1>'.$output;
  }
  return $output;
}


function frontpage_more($start) {
  $nodes  = frontpage_getData(30,$start);
  $output = theme ('table', array(), $nodes);
  $output = preg_match('|<tbody[^>]*?>(.*?)</tbody>|si',$output,$match);
  print $match[1];
}

/**
 * Fire SQL and get Data
 * 
 * @param integer $limit Wie viele Einträge maximal geholt werden sollen
 * @param integer $start Startwert der Pagination
 * @return array 
 */
function frontpage_getData($limit=10, $start=0) {
  $nodes = array();
  $sql = variable_get('frontpage_sql', '');
  $res = db_query(db_rewrite_sql($sql),$start,$limit);
  while ($r = db_fetch_array($res)) {
    $node = new stdClass();
    $node->nid = $r['nid'];
    $node->type = variable_get('frontpage_dossiertype', 'dossier');
    $nodes[$r['nid']]['type']       = _searchtools_nodetypeIcon($r['node_type']);
    $nodes[$r['nid']]['titel']      = l($r['node_title'],'node/'.$r['nid'])._searchtools_hasCoordinates($r['nid']);
    $nodes[$r['nid']]['changed']    = _searchtools_ago($r['node_changed']);
    $nodes[$r['nid']]['change_uid'] = l(_searchtools_uid2username($r['change_uid']), 'user/'.$r['change_uid']);
    $nodes[$r['nid']]['og']         = _searchtools_getGroupIcons(og_get_node_groups($node), _searchtools_og_isPublic($r['nid']));
    $nodes[$r['nid']]['revision']   = $r['log'];
  }
  return $nodes;
}