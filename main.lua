function PrintString()
print([[
   _    __   ___  _____ _____     _        _    
  /_\  / _\ / __\ \_   \\_   \   /_\  _ __| |_  
 //_\\ \ \ / /     / /\/ / /\/  //_\\|  __| __| 
/  _  \_\ | /___/\/ /_/\/ /_   /  _  \ |  | |_  
\_/ \_/\__|____/\____/\____/   \_/ \_/_|   \__| 
]])
end


table =
{
	var1 = 5,
	var2 =
	{
		var3 = 66;
	}
}

function main()
	testing = Counter(10,"countingName");
	print(testing:GetValue());
	testing:SetValue(6);
	print(testing:GetValue());
	print(testing:PrintName());
end