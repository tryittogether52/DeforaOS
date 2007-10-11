<div class="pki ca">
	<h1 class="title caclient"><?php echo _html_safe($caclient['title']); ?></h1>
	<table>
		<tr><td class="field"><?php echo _html_safe(COUNTRY); ?>:</td><td><?php echo _html_safe($caclient['country']); ?></td></tr>
		<tr><td class="field"><?php echo _html_safe(STATE); ?>:</td><td><?php echo _html_safe($caclient['state']); ?></td></tr>
		<tr><td class="field"><?php echo _html_safe(LOCALITY); ?>:</td><td><?php echo _html_safe($caclient['locality']); ?></td></tr>
		<tr><td class="field"><?php echo _html_safe(ORGANIZATION); ?>:</td><td><?php echo _html_safe($caclient['organization']); ?></td></tr>
		<tr><td class="field"><?php echo _html_safe(SECTION); ?> (OU):</td><td><?php echo _html_safe($caclient['section']); ?></td></tr>
		<tr><td class="field"><?php echo _html_safe(COMMON_NAME); ?> (CN):</td><td><?php echo _html_safe($caclient['cn']); ?></td></tr>
		<tr><td class="field"><?php echo _html_safe(EMAIL); ?>:</td><td><?php echo _html_safe($caclient['email']); ?></td></tr>
	</table>
	<div class="toolbar"><form action="index.php" method="post"><input type="hidden" name="module" value="pki"/><input type="hidden" name="action" value="caclient_export"/><input type="hidden" name="id" value="<?php echo _html_safe($caclient['id']); ?>"/><?php echo _html_safe(KEY); ?>: <input type="password" name="key" value=""/> <input type="submit" value="<?php echo _html_safe(EXPORT); ?>"/></form></div>
</div>
