		<item>
			<title><?php echo _html_safe($news['title']); ?></title>
			<author><?php echo _html_safe($news['author']); ?></author>
			<pubDate><?php echo _html_safe($news['date']); ?></pubDate>
			<link><?php echo _html_safe($news['link']); ?></link>
			<guid><?php echo _html_safe($news['link']); ?></guid>
			<description><![CDATA[<?php echo $news['content']; ?>]]></description>
		</item>
