// $Id: microstrategy.js,v 1.0.0.0 2010/10/19 18:24:24 psynaptic Exp $

Drupal.behaviors.microstrategy = function (context) {
  if (Drupal.settings.microstrategy.hover_links) {
    $('.mstr-edit-link').hide();
    $('div.block').mouseover(function() {
      $(this).find('.mstr-edit-link').css('display', 'block');
    });

    $('div.block').mouseout(function() {
      $(this).find('.mstr-edit-link').css('display', 'none');
    });
  };
};
