<?php
/**
 * Gibt ein JSON Objekt mit den gefilterten Resultate zurück
 */
function _searchtools_results_json($type,$query) {
  include_once('search.inc');
  include_once('elements.inc');
  include_once('functions.inc');
  
  $nodes = array();
  drupal_set_header('Content-Type: text/plain; charset: utf-8');
  $nids = _dossiersearch_getresults($query);
  foreach ($nids as $key=>$nid) {
    $terms = array();
    $node = node_load($nid);
    foreach ($node->taxonomy as $term) {
      $terms[] = l($term->name,'taxonomy/term/' . $term->tid);
    }
    $nodes[$node->changed.':'.$key] = array(
        'nid'       => $nid,
        'title'     => $node->title,
        'autor'     => $node->name,
        'changed'   => date('d.m.Y',$node->changed),
        'date'      => date('d.m.Y',strtotime($node->field_dokumentendatum[0]['value'])),
        'uid'       => $node->uid,
        'location'  => array(
            'latitude'  => $node->locations[0]['latitude'],
            'longitude' => $node->locations[0]['longitude'],
        ),
        'groups'    => _searchtools_getGroupIcons($node->og_groups_both),
        'terms'     => implode(', ',$terms),
        'filescount'=> count($node->field_dateien),
        'thumbs'    => ($type=='Thumbnails')?_previews_getcontent($node,'aus: '.l($node->title,'node/'.$node->nid)):'',
    );
  }
  krsort($nodes);
  print drupal_to_js($nodes);
}