(function() {

    var curPos = 0;
    var query = '';
    var timeout;

    if (Drupal.jsEnabled) {
        $(document).ready(function() {
            
            curPos = Drupal.settings.frontpage.initialEntryCount;
            
            // Suchfeld unsichtbar hinzufügen
            $('body').append('<div id="frontpage_query"></div>');

            // Infinite-Scrolling Event
            $(window).scroll(function() {
                if ($(window).scrollTop() == $(document).height() - $(window).height()) {
                    more(20);
                }
            });
            
            // Wenn keine Scrollbars (z.B. bei grossem Bildschirm),
            // dann mehr Einträge nachladen
            if ($("body").height() <= $(window).height()) {
                more(30);
            }
        });

        // Buchstaben-Suche
        $(document).keypress(function(e) {
            if (!ignore(e.target.id)) {
                window.clearTimeout(timeout);
                var key = e.which;
                if (key >= 32 && query.length < 22) {
                    query += String.fromCharCode(key);
                    $('#frontpage_query').html('nach "' + query + '" suchen [Enter]');
                    $('#frontpage_query').show();
                    timeout = window.setTimeout(hideInput, 5000); // Input Feld wird nach Timeout wieder ausgeblendet
                    return false;
                }
                else return true;
            }
            return true;
        });

        // Sonderzeichen mit keydown!
        $(document).keydown(function(e) {
            if (!ignore(e.target.id)) {
                var key = e.which;
                // Enter: Wenn query nicht leer, dann Suche starten
                if (key === 13 && query !== '') {
                    e.preventDefault();
                    window.location = '/dossier/filter/' + query;
                    return false;
                }
                // Backspace: Eingabe löschen
                else if (key === 8 && query !== '') {
                    e.preventDefault();
                    query = query.substring(0, query.length - 1);
                    $('#frontpage_query').html('nach "' + query + '" suchen [Enter]');
                    return false;
                }
                else if (key == 8) { // Wenn query = '' und Löschen-Taste, dann nichts machen
                    e.preventDefault();
                    return false;
                }
                // Escape: Inputfeld schliessen
                else if (key == 27) {
                    hideInput();
                }
                else return true;
            }
            return true;
        }); 
    }

    /**
     * Checked, ob Cursor in nicht-Suchfeld ist
     */
    var ignore = function(keyId) {
        var ignoreList = ['edit-search-theme-form-1', 'edit-masquerade-user-field'];
        return (ignoreList.indexOf(keyId) === -1) ? false : true;
    };



    /**
     * Eingabefeld ausblenden 
     */
    var hideInput = function() {
        $('#frontpage_query').fadeOut();
        query = '';
    };


    /**
     * Einträge unten an die Liste anhängen
     * Diese Funktion wird aufgerufen, wenn der Benutzer
     * nach unten gescrollt hat und Einräge nachgeladen
     * werden sollen.
     */
    var more = function(count) {
        $.get('Neustes/more/' + curPos + '/' + count, function(data) {
            $('#frontpage_table tbody').append(data);
            curPos += count;
        });
    };

})();