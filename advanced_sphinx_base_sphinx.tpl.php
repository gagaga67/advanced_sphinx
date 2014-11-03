<?php $fields_txt = advanced_sphinx_get_fieldstype();
  foreach ($fields_txt as $ntype => $value) {
    $c = count($value);
    $n = 0;
    $res = '';
    while ($n < $c) {
      $res = $res . ', ' . $value[$n] . '_value';
      $n++;
    }

?>
source source_<?php print $ntype; ?>
{
  type               = mysql
  sql_host           = localhost
  sql_user           = <?php print $config['sql_user']; ?>
  sql_pass           = <?php print $config['sql_pass']; ?>
  sql_db             = <?php print $config['sql_db']; ?>
  sql_port           = 3306
  sql_query_pre      = SET NAMES utf8
  sql_query_pre      = SET CHARACTER SET utf8
  sql_query_pre      = REPLACE INTO advanced_sphinx_livup SELECT 1, MAX(nid) FROM node
  sql_query          = SELECT nid, nid as node_id, title, created, changed<?php print $res; ?>, countitl, CRC32(type) as type, CRC32(language) as language FROM `<?php print 'Sphinxindex_'.$ntype; ?>`
  sql_attr_uint      = node_id
  sql_attr_uint      = countitl
  sql_attr_uint      = type
  sql_attr_uint      = language
  sql_attr_timestamp = created
  sql_attr_timestamp = changed
  sql_query_info     = SELECT * FROM <?php print 'Sphinxindex_'.$ntype; ?> WHERE nid = $id
}

index index_<?php print $ntype; ?>
{
  source			 = source_<?php print $ntype . " \n"; ?>
  path				 = <?php print $config['dir'] . '/index/' . $ntype . " \n"; ?>
  docinfo			 = extern
  morphology		 = stem_enru
  charset_type		 = utf-8
  charset_table		 = <?php print $config['charset_table']; ?>
  min_word_len		 = <?php print $config['min_word_len']; ?>
  min_infix_len      = <?php print $config['min_infix_len']; ?>
  enable_star        = <?php print $config['enable_star']; ?>
  html_strip		 = <?php print $config['html_strip']; ?>
  wordforms          = <?php print $config['wordforms']; ?>
  exceptions         = <?php print $config['exceptions']; ?>
  expand_keywords    = <?php print $config['expand_keywords']; ?>
}
<?php } ?>

index index_main_join
{
  type					= distributed
<?php foreach ($fields_txt as $ntype => $value) { ?>  
  local					= index_<?php print $ntype . " \n"; ?>
<?php } ?>
}

indexer
{
  mem_limit			 = 32M
}

searchd
{
  listen             = <?php print $config['listen']; ?>
  log				 = <?php print $config['log']; ?>
  query_log		     = <?php print $config['query_log']; ?>
  read_timeout	     = 5
  max_children	     = 30
  pid_file		     = <?php print $config['searchd']; ?>
  max_matches		 = 1000
  seamless_rotate	 = 1
  preopen_indexes	 = 1
  unlink_old		 = 1
}