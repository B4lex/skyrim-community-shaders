namespace CharacterOutline
{
	float GetPlayerMask()
	{
		bool isPlayer = (Permutation::ExtraShaderDescriptor & Permutation::ExtraFlags::IsPlayerCharacter) != 0;
		bool isNPC = (Permutation::ExtraShaderDescriptor & Permutation::ExtraFlags::IsNPC) != 0;
		return (isPlayer || isNPC) ? 1.0 : 0.0;
	}
}