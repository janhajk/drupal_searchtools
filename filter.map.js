// Zentrum der Karte
var dossiersearch_zentrum_name = Drupal.settings.dossiersearch.center.title;



if(Drupal.jsEnabled) {
   $(document).ready(function() {

      if (!dossiersearch_usemap) { $('#dossiersearch_filter_location').hide(); }
      //$('#dossiersearch_filter_group_select').attr("disabled", true);

      if(dossiersearch_showmap && dossiersearch_usemap) {
         $('#dossiersearch_gmap').show();
         gmap_initialize();
         dossiersearch_displayProjekt();
         $('#dossiersearch_filter_map_active').attr('checked', false);
      }
      $('#dossiersearch_filter_map_active').change(function() {
         if($('#dossiersearch_filter_map_active').attr('checked')) {
            if(firsttime && !dossiersearch_showmap) {
               $('#dossiersearch_gmap').show();
               gmap_initialize();
               dossiersearch_displayProjekt();
            }
         }
         dossiersearch_search_init(0);
      });
   });
}

var map;
function initMap() {
  map = new google.maps.Map(document.getElementById('dossiersearch_filter_location'), {
    center: {lat: Drupal.settings.dossiersearch.center.latitude, lng: Drupal.settings.dossiersearch.center.longitude},
    zoom: 8
  });
}




/*
 * Places Marks on the Map for every Node
 */
function dossiersearch_nodeMarks(nid,position) {
   if (dossiersearchMarkers[nid] != undefined) {
      dossiersearchMarkers[nid].setMap(null);
   }
   dossiersearchMarkers[nid] = new google.maps.Marker({
      position: position,
      map: dossiersearchMap,
      icon: '/sites/all/modules/custom/drupal_searchtools/images/bluedot.png'
   });
   google.maps.event.addListener(dossiersearchMarkers[nid], 'click', function() {
      window.location = '/node/' + nid;
   });
}

// Gmap darstellen
function gmap_initialize() {
   if (!dossiersearch_usemap) return false;
   firsttime = false;
   var myOptions = {
      zoom: 14,
      center: dossiersearch_zentrum,
      mapTypeId: google.maps.MapTypeId.HYBRID,
      navigationControl: true,
      navigationControlOptions: {style: google.maps.NavigationControlStyle.SMALL},
      mapTypeControl: false,
      scaleControl: true,
      streetViewControl: false
   }

   dossiersearchMap = new google.maps.Map(document.getElementById("dossiersearch_gmap"), myOptions);

   google.maps.event.addListener(dossiersearchMap, 'bounds_changed', function() {
      document.getElementById('dossiersearch_filter_bounds').innerHTML = dossiersearchMap.getBounds();
      if ($('#dossiersearch_filter_map_active').attr('checked')) {
         dossiersearch_search_init(1500);
      }
      dossiersearch_updateExport();
   });

   // erzeugt einen "Home"-Button
   var homeControlDiv = document.createElement('DIV');
   homeControlDiv.style.padding = '5px';
   var controlUI = document.createElement('DIV');
   controlUI.id = 'dossiersearch_filter_map_home_button';
   controlUI.title = 'Hier clicken um zurück nach ' + dossiersearch_zentrum_name + ' zu kommen';
   homeControlDiv.appendChild(controlUI);
   var controlText = document.createElement('DIV');
   controlText.id = 'dossiersearch_filter_map_home_text';
   controlText.innerHTML = dossiersearch_zentrum_name;
   controlUI.appendChild(controlText);
   google.maps.event.addDomListener(controlUI, 'click', function() {
      dossiersearchMap.panTo(dossiersearch_zentrum);
      dossiersearchMap.setOptions({zoom:14});
   });
   homeControlDiv.index = 1;
   dossiersearchMap.controls[google.maps.ControlPosition.TOP_RIGHT].push(homeControlDiv);
   // Ende Home-Button

   return true;
}


/*
 * Polygon für Projektgebiet
 * die einzelnen Punkte koennen im Dossiersearch-Admin eingegeben werden
 */
function dossiersearch_displayProjekt()  {
   var projektgebiet = Drupal.settings.dossiersearch.projekt.map;
   var gProjektgebiet = [];
   for (var latlng in projektgebiet) {
      gProjektgebiet.push(new google.maps.LatLng(projektgebiet[latlng][0],projektgebiet[latlng][1]));
   }
   var projectlines = new google.maps.Polygon({
      path: gProjektgebiet,
      strokeColor: "#FF0000",
      strokeOpacity: 0.8,
      strokeWeight: 2,
      fillColor: "#FF0000",
      fillOpacity: 0.35
   });
   projectlines.setMap(dossiersearchMap);  
}