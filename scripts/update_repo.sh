function update_repo()
{
	#ssh uhururepo 'cd ~/Uhuru_Repository/Uhuru/repo/debian && reprepro processincoming wheezy'
	#ssh uhururepo 'cd ~/Uhuru_Repository/Uhuru/repo/ubuntu && reprepro processincoming precise'
	ssh uhururepo 'cd ~/Uhuru_Repository/Uhuru/repo/ubuntu && reprepro processincoming trusty'
}

update_repo
