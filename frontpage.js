var frontpage_start = 30;
var frontpage_query = '';
var frontpage_timeout;

Array.prototype.in_array = function(needle) {
for(var i=0; i < this.length; i++) if(this[ i] === needle) return true;
return false;
}

if(Drupal.jsEnabled) {
  $(document).ready(function() {
      $('body').append('<div id="frontpage_query"></div>');
      $(window).scroll(function(){
        if  ($(window).scrollTop() == $(document).height() - $(window).height()){
          frontpage_loadMore();
        }
      });
  });
  // Buchstaben-Suche
  $(document).keypress(function(e){
    if (e.target.id != 'edit-search-theme-form-1' && e.target.id != 'edit-masquerade-user-field') {
      window.clearTimeout(frontpage_timeout);
      var key = e.which;
      if (key >= 32 && frontpage_query.length < 22) {
        frontpage_query = frontpage_query + String.fromCharCode(key);
        $('#frontpage_query').html('nach "' + frontpage_query + '" suchen [Enter]');
        $('#frontpage_query').show();
        frontpage_timeout = window.setTimeout('frontpage_hide()',5000);
        return false;
      }
      else return true;
    }
    return true;
  });
  // Sonderzeichen mit keydown!
  $(document).keydown(function(e){
    if (e.target.id != 'edit-search-theme-form-1' && e.target.id != 'edit-masquerade-user-field') {
      var key = e.which;
      // Enter Drücken
      if (key == 13 && frontpage_query != '') {
        e.preventDefault();
        window.location = '/' + Drupal.settings.frontpage.searchpath + '/'+frontpage_query;
        return false;
      }
      // Backspace
      else if (key == 8 && frontpage_query != '') {
        e.preventDefault();
        frontpage_query = frontpage_query.substring(0,frontpage_query.length-1);
        $('#frontpage_query').html('nach "' + frontpage_query + '" suchen [Enter]');
        return false;
      }
      else if (key == 8) {
        e.preventDefault();
        return false;
      }
      // Escape
      else if (key == 27) {
        frontpage_hide();
      }
      else return true;
    }
    return true;
  });
}


function frontpage_hide() {
  $('#frontpage_query').fadeOut();
  frontpage_query = '';
}

var frontpage_loadMore = function() {
  $.get('start/more/' + frontpage_start, function(data) {
    $('#frontpage_table tbody').append(data);
    frontpage_start = frontpage_start + 30;
  });
};