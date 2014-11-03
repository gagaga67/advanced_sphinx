<?php
// $Id: advanced_sphinx_items_result_main.tpl.php, v 1.0 2011/07/05 19:59:16 gagaga Exp $
/**
 * @file advanced_sphinx_items_result_main.tpl.php
 * Default theme implementation to item search result.
 *
 * Available variables:
 * - $result: Array with all data:
 *   - $result['number']: Serial number search results.
 *   - $result['title']: Linked title to full node.
 *   - $result['excerpts']: .
 *   - $result['date']: Date and time of posting.
 *   - $result['username']: Linked login to node author.
 *   - $result['tax']: List of taxonomy term.
 *   - $result['comment_count']: Comment count on this node.
 *   - $result['file']: File count on this node.
 */
?>
<li class="result-folded">
  <h3 class="title-result"><span class="number-result"><?php print $result['number']; ?>.</span> <?php print $result['title']; ?></h3>
  <div class="content-result">
    <?php print $result['excerpts']; ?>
  </div>
  <div class="info-result">
    <span class="date-result"><?php print format_date($result['date'], 'custom','d.m.Y'); ?>,</span>
    <span class="autor-result"><?php print $result['username']; ?>,</span>
    <span class="type-result"><?php print $result['type']; ?></span>
    <span class="tax-result"><?php print t('Tags') .': '.$result['tax']; ?></span>
  </div>
</li>
