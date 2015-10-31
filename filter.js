var dossiersearch_showmap = Drupal.settings.dossiersearch.gmap.show;
var dossiersearch_usemap  = Drupal.settings.dossiersearch.gmap.use;

// ID des Counters um Verzögerung für ajax abfragen bei Live-Suche zu erzeugen
var dossiersearchDelayId = 0;

// googlemap
var dossiersearchMap;

// markers
var dossiersearchMarkers = [];

var dossiersearch_nodeMarks = function(){};

var firsttime = true;
var initSorter = true;

if(Drupal.jsEnabled) {
   $(document).ready(
      function() {
         //$('#dossiersearch_results table').tablesorter({headers: {1:{sorter:'germandate'}}});
         dossiersearch_addDateFilter();

         // Initial Suche
         dossiersearch_search_init(0);

         // Filtert liste wenn Text in Titel-Filter eingegeben wurde
         $('#dossiersearch_filter_title').keyup(function() {
            dossiersearch_search_init();
         });

         // Filtert Liste nach Taxonomy Select Change
         $('#dossiersearch_filter_projektgebiet_select,'+
           '#dossiersearch_filter_fachgebiet_select,'+
           '#dossiersearch_filter_dateityp_select,'+
           '#dossiersearch_filter_group_select').change(function(){
            dossiersearch_search_init(0);
         });

         $('#dossiersearch_view img').click(function() {
            var view = dossiersearch_getCurrentView();
            view = (view === 'Normal') ? 'Thumbnails' : 'Normal';
            $('#dossiersearch_view img').attr('dossiersearch_view',view);
            $('#dossiersearch_view img').attr('src',Drupal.settings.dossiersearch.path + '/images/view_' + view + '.png');
            dossiersearch_search_init(0);
         });
      }
   );
}

/*
 * Initiert die Suche
 *
 * Die Suche wird innerhalb eines timeout's nur einmal durchgeführt
 * ist bereits ein timeout am laufen, wird es erneuert und die Suche
 * wird erneut erst nach dem Ablauf des timeout's ausgeführt
 */
function dossiersearch_search_init(timeout) {
   // Default Wert, wenn nicht gesetzt
   timeout = typeof(timeout) != 'undefined' ? timeout : 1000;
   // Wenn Timer bereits läuft, dann reseten
   if (dossiersearchDelayId) {
      clearTimeout(dossiersearchDelayId);
   }
   // wenn timeout = 0 dann soll er kein Timer setzten, sondern direkt Funktion aufrufen
   if (timeout == 0) {
      dossiersearch_search();
   }
   else {
      // Timeout setzten
      dossiersearchDelayId = window.setTimeout('dossiersearch_search()', timeout);
      // Export Link für XLS-Export aktualisieren
   }
   dossiersearch_updateExport();
   return true;
}

/*
 * Sammelt alle Teilbedingungen/Filter für die Query
 */
function dossiersearch_getQuery() {
   var query = 'title='       + $('#dossiersearch_filter_title').val();
   query +=  ';bounds='       + $('#dossiersearch_filter_bounds').html();
   query += ';mapactive='     + $('#dossiersearch_filter_map_active').attr('checked');
   query += ';group='         + $('#dossiersearch_filter_group_select').val();
   query += ';date='          + $("#dossiersearch_filter_date_from").val()+','+$("#dossiersearch_filter_date_to").val();
   query += ';terms='         + $('#dossiersearch_filter_fachgebiet_select').val();
   query += ','               + $('#dossiersearch_filter_dateityp_select').val();
   query += ','               + $('#dossiersearch_filter_projektgebiet_select').val();
   return query;
}

/*
 * Suche ausführen und Resultate ausgeben
 */
function dossiersearch_search() {
   var query = dossiersearch_getQuery();
   var view = dossiersearch_getCurrentView();                        // aktuelle View (Thumbnails oder Normal)

   // Fire AJAX-Search
   $.getJSON('/dossier/filter/results/'+view+'/'+query, function (data) {
      dossiersearchNodes = data;                                      // Suchresultate in Globaler Variable speichern
      var results = dossiersearch_themeResults(view,data);          // Resultate in HTML Formatieren
      $('#dossiersearch_results_count').html('Anzahl gefundene Dossiers: ' + results[0]);
      $('#dossiersearch_results_body').empty();                // Suchresultate ausblenden und danach leeren
      $('#dossiersearch_results table').trigger("reset");
      dossiersearch_setHeader(view);                                  // Header der Tabelle aendern
      $('#dossiersearch_results table tbody').html(results[1]);
      $('#dossiersearch_results_body').css('text-align','left').fadeIn(); // Resultate anzeigen
      previews_init();
   });
   return true;
}

/*
 * aktualisiert bei jeder filter-änderung den Link des CSV-Export-Buttons
 */
function dossiersearch_updateExport() {
   var query = dossiersearch_getQuery();
   $('#dossiersearch_export a').attr('href','/dossier/filter/export/' + dossiersearch_base64encode(query));
}


/**
 * liest die Aktuelle Ansicht (Thumbnails oder Normal)
 */
function dossiersearch_getCurrentView() {
   var view = $('#dossiersearch_view img').attr('dossiersearch_view');
   return (view === 'Thumbnails') ? view : 'Normal';
}


/*
 * Themeing Funktion für Suchresultate
 * wird gemäss der aktuellen VIew ausgeührt für Thumbnails oder Normal (Listenansicht)
 */
function dossiersearch_themeResults(view,data) {
   var rows = '';
   var count = 0;

   for (var key in data) {
      if (view === 'Thumbnails') {
         if (data[key]['thumbs'] != '') {
            rows += '<tr><td>' + data[key]['thumbs'] + '</td></tr>';
         }
      }
      else {
         rows += '<tr><td><a href="/node/' + data[key]['nid'] + '">' + data[key]['title'] + '</a></td>'+
            '<td>' + data[key]['changed'] + '</td>'+
            '<td>' + data[key]['date'] + '</td>'+
            '<td><a href="/user/' + data[key]['uid'] + '">' + data[key]['autor'] + '</a></td>'+
            '<td>[' + data[key]['filescount'] + ']</td>'+
            '<td>' + data[key]['groups'] + '</td>'+
            '<td>' + data[key]['terms'] + '</td>'+
            '</tr>';
      }
      // Marker des Nodes auf Karte setzten
      dossiersearch_nodeMarks(
         data[key]['nid'],
         data[key]['location']['latitude'],
         data[key]['location']['longitude']
      );
      count++;
   }
   return new Array(count,rows);
}

/*
 * setzt den Tabellenkopf für die Suchresultate gemäss der View (Normal oder Thumbnails)
 */
function dossiersearch_setHeader(view) {
   var header = (view === 'Thumbnails') 
   ? '<tr><th>Titel</th></tr>'
   : '<tr><th>Titel</th><th>letzte &Auml;nderung</th><th>Dok-Datum</th><th>Autor</th><th>Dateien</th><th>Gruppen</th><th>Tags</th></tr>';
   $('#dossiersearch_results_head').html(header);
}


/*
 *
 */
function dossiersearch_addDateFilter() {
   //$('#dossiersearch_filter_date').prepend();
   $('#dossiersearch_filter_date_slider').slider({
      range:  true,
      min:    0,
      max:    Drupal.settings.dossiersearch.projekt.range-1,
      values: [0,Drupal.settings.dossiersearch.projekt.range-1],
      slide:  function( event, ui ) {
         var from = ui.values[0]*60*60*24 + Drupal.settings.dossiersearch.projekt.mindate;
         var to   = ui.values[1]*60*60*24 + Drupal.settings.dossiersearch.projekt.mindate;
         $("#dossiersearch_filter_date_value").html(phpdate(from) + " - " + phpdate(to));
         $("#dossiersearch_filter_date_from").val(from);
         $("#dossiersearch_filter_date_to").val(to);
      },
      change: function() {
         dossiersearch_search_init(0);
      }
   });
   $("#dossiersearch_filter_date_value").html(
      phpdate(Drupal.settings.dossiersearch.projekt.mindate) +
      " - " +
      phpdate(Drupal.settings.dossiersearch.projekt.maxdate));
}

/*
 * base64 encoder function
 */
function dossiersearch_base64encode(inp){
   var key="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
   var chr1,chr2,chr3,enc3,enc4,i=0,out="";
   while(i<inp.length){
      chr1=inp.charCodeAt(i++);if(chr1>127) chr1=88;
      chr2=inp.charCodeAt(i++);if(chr2>127) chr2=88;
      chr3=inp.charCodeAt(i++);if(chr3>127) chr3=88;
      if(isNaN(chr3)) {enc4=64;chr3=0;} else enc4=chr3&63
      if(isNaN(chr2)) {enc3=64;chr2=0;} else enc3=((chr2<<2)|(chr3>>6))&63
      out+=key.charAt((chr1>>2)&63)+key.charAt(((chr1<<4)|(chr2>>4))&63)+key.charAt(enc3)+key.charAt(enc4);
   }
   return encodeURIComponent(out);
}

function phpdate(timestamp) {
   var Zeit = new Date(timestamp*1000);
   var day  = '0' + Zeit.getDate();
   return day.substr(day.length-2,2) + '.' + (Zeit.getMonth()+1) + '.' + Zeit.getFullYear();
}