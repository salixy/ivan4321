# Helper script to create GitHub repository
$token = $env:GITHUB_TOKEN
if (-not $token) {
    Write-Host "Please set GITHUB_TOKEN environment variable."
    exit 1
}
$headers = @{
    "Authorization" = "token $token"
    "User-Agent"    = "Antigravity"
    "Accept"        = "application/vnd.github.v3+json"
}
$body = @{
    name        = "ivan4321"
    description = "ImGui C++ Application"
    private     = $false
} | ConvertTo-Json

try {
    $res = Invoke-RestMethod -Uri "https://api.github.com/user/repos" -Method Post -Headers $headers -Body $body
    Write-Host "Repository created successfully: $($res.html_url)"
} catch {
    Write-Host "Error creating repository: $_"
}
